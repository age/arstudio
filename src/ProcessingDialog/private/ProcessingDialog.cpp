#include <ProcessingDialog.hpp>

ProcessingDialog::ProcessingDialog (QWidget * parent) :
	QWidget (parent)
{
	createLayout ();

	connect (select_file_button, SIGNAL (clicked ()), this,
		SLOT (select_file ()));
	connect (process_button, SIGNAL (clicked ()), this, SLOT (process_frames ()));
	connect (this, SIGNAL (progress_signal ()), this, SLOT (update_progress ()));
	connect (radio_select_frames, SIGNAL (toggled (bool)), this,
		SLOT (select_frames_changed (bool)));
	connect (stop_button, SIGNAL (clicked ()), this, SLOT (stop_clicked ()));

	connect (this, SIGNAL (processing_done (bool, const std::string &)), this,
		SLOT (processing_cleanup (void)));

	// defined in CMakeLists.txt
	populateConfig ("@CONFIG_DIRECTORY@");

	lastSelectedFile =
	  settings.value ("ProcessingDialog/lastSelectedFile", "~").toString ();
	if (lastSelectedFile != "~")
		{
			loadFile (lastSelectedFile.toStdString ());
		}
}

void
ProcessingDialog::processing_cleanup (void)
{
	unlockUI ();
	this->hide ();
}

ProcessingDialog::~ProcessingDialog ()
{
	settings.setValue ("ProcessingDialog/lastSelectedFile", lastSelectedFile);
}

void
ProcessingDialog::createLayout ()
{
	file_name_label->setText ("File: ");
	frames_count_label->setText ("Frames: ");
	select_file_button->setText ("Select file");
	process_button->setText ("Process");
	stop_button->setText ("Stop");

	layout->addWidget (file_name_label, 0, 0, 1, 3, 0);
	layout->addWidget (frames_count_label, 1, 0, 1, 3, 0);
	layout->addWidget (radio_whole_file, 2, 0);
	layout->addWidget (radio_select_frames, 3, 0);
	layout->addWidget (start_frame_spin, 3, 1, 1, 1, 0);
	layout->addWidget (end_frame_spin, 3, 2, 1, 1, 0);
	layout->addWidget (progress_bar, 4, 0, 1, 3, 0);
	layout->addWidget (select_file_button, 5, 0, 1, 1, 0);
	layout->addWidget (process_button, 5, 1, 1, 1, 0);
	layout->addWidget (stop_button, 5, 2, 1, 1, 0);
	layout->addWidget (editor, 0, 3, 6, 8);

	lockUI ();
	select_file_button->setEnabled (true);
	stop_button->setEnabled (false);

	progress_bar->setFormat ("%v / %m");
	progress_bar->setValue (0);
	progress_bar->setMaximum (-1);
}

/**
 *      This method imports settings from every directory at path, by
 * convention
 *      -- from a file named "settings.xml".
 *
 *      \param path the directory to scan for sub-directories
 */

void
ProcessingDialog::populateConfig (std::string path)
{
	boost::filesystem::directory_iterator dir (path), end;

	for (; dir != end; dir++)
		{
			boost::filesystem::path p = dir->path ();
			if (boost::filesystem::is_directory (p))
				{
					config->import_xml (p.string () + "/settings.xml");
				}
		}
}

void
ProcessingDialog::select_frames_changed (bool state)
{
	start_frame_spin->setEnabled (state);
	end_frame_spin->setEnabled (state);
}

void
ProcessingDialog::stop_clicked (void)
{
	run_thread = false;
}

void
ProcessingDialog::select_file ()
{
	selectedFile = QFileDialog::getOpenFileName (this, tr ("Open Video"),
		lastSelectedFile, tr ("Video Files (*.avi *.mkv *.wmv *.mp4 *.kinvideo)"));
	if (selectedFile.isNull ())
		{
			return;
		}
	lastSelectedFile = selectedFile;

	loadFile (selectedFile.toStdString ());
}

/**
 *      This method loads a video file either with OpenCV VideoCapture, or
 * with
 *      FileCapture for kinvide.
 *      \param path path to the file
 */

bool
ProcessingDialog::loadFile (std::string path)
{
	QFileInfo fileInfo (QString::fromStdString (path));

	if (fileInfo.isFile () == false)
		{
			return false;
		}
	unsigned int frames_count;

	cv::VideoCapture * c;
	file_name_label->setText ("File: " + fileInfo.baseName ());
	file_name_label->setToolTip (selectedFile);
	radio_whole_file->setEnabled (true);
	radio_whole_file->setChecked (true);
	process_button->setEnabled (true);

	if (kincap)
		{
			delete kincap;
			kincap = nullptr;
		}
	else if (vcap)
		{
			delete vcap;
			vcap = nullptr;
		}

	if (fileInfo.suffix () == "kinvideo")
		{
			kincap       = new FileCapture (path);
			frames_count = kincap->getFrameCount ();
		}
	else
		{
			c = new cv::VideoCapture (path);
			if (!c->isOpened ())
				{
					frames_count = -1;
					vcap         = NULL;
				}
			else
				{
					frames_count = c->get (CV_CAP_PROP_FRAME_COUNT);
					start_frame_spin->setRange (0, frames_count - 1);
					end_frame_spin->setRange (1, frames_count);
					vcap = c;
					radio_select_frames->setEnabled (true);
				}
		}
	frames_count_label->setText ("Frames: " + QString::number (frames_count));
	return true;
}

/**
 *      This method determines the range of frames to be processed and
 * starts the processing thread.
 */

void
ProcessingDialog::process_frames (void)
{
	if (!vcap && !kincap)
		{
			return;
		}

	int start, end;
	if (radio_whole_file->isChecked ())
		{
			start = 0;
			if (vcap)
				{
					end = vcap->get (CV_CAP_PROP_FRAME_COUNT);
				}
			else
				{
					end = kincap->getFrameCount ();
				}
		}
	else
		{
			start = start_frame_spin->value ();
			end   = end_frame_spin->value ();
		}

	if (start >= end)
		{
			return;
		}
	progress_bar->setRange (0, end - start);
	progress_bar->setValue (1);

	lockUI ();
	clearRepository ();

	run_thread = true;

	algo_pipeline = AlgoPipeline::make (config);
	try
		{
			algo_pipeline->create_all ();
		}
	catch (const std::runtime_error & error)
		{
			processing_done (false, error.what ());
			return;
		}

	QFuture <void> thread = QtConcurrent::run (this,
		&ProcessingDialog::processing_thread, start, end);
}

void
ProcessingDialog::processing_thread (int start, int end)
{
	cv::Mat image, empty;

	if (vcap)
		{
			vcap->set (CV_CAP_PROP_POS_FRAMES, start);
		}
	for (int i = start; i < end && run_thread; i++)
		{
			try
				{
					if (vcap)
						{
							*vcap >> image;
							algo_pipeline->process_frame (image, empty);
						}
					else
						{
							kincap->readFrame ();
							LuxFrame * f = kincap->getFrame ();
							algo_pipeline->process_frame (f->image, f->depth_map);
						}
				}
			catch (const std::out_of_range & error)
				{
					processing_done (false, error.what ());
					return;
				}
			progress_signal ();
		}
	processing_done (true, std::string ());
}

void
ProcessingDialog::update_progress (void)
{
	progress_bar->setValue (progress_bar->value () + 1);
}

void
ProcessingDialog::lockUI ()
{
	radio_whole_file->setEnabled (false);
	radio_select_frames->setEnabled (false);
	start_frame_spin->setEnabled (false);
	end_frame_spin->setEnabled (false);
	process_button->setEnabled (false);
	select_file_button->setEnabled (false);
	stop_button->setEnabled (true);
}

void
ProcessingDialog::unlockUI ()
{
	process_button->setEnabled (true);
	select_file_button->setEnabled (true);
	stop_button->setEnabled (false);
	radio_whole_file->setEnabled (true);
	radio_select_frames->setEnabled (true);
	start_frame_spin->setEnabled (true);
	end_frame_spin->setEnabled (true);
}