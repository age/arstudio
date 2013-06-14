#include <VideoPipeline.hpp>

namespace arstudio {
VideoPipeline::VideoPipeline (QObject * parent)
  : QObject (parent),
  m_run_processing (false),
  m_video_helper (nullptr),
  m_config (nullptr),
  m_start_frame (1),
  m_end_frame (1),
  m_processing_progress (0)
{
}

bool
VideoPipeline::running (void)
{
  return m_run_processing;
}

void
VideoPipeline::set_running (bool v)
{
  m_run_processing = v;
  running_changed ();
  if (v)
    QtConcurrent::run (this, &VideoPipeline::processing_thread);
}

VideoHelper *
VideoPipeline::video_helper (void)
{
  return m_video_helper;
}

void
VideoPipeline::set_video_helper (VideoHelper * vh)
{
  m_video_helper = vh;
}

Config *
VideoPipeline::config ()
{
  return m_config;
}

void
VideoPipeline::set_config (Config * ptr)
{
  m_config = ptr;
}

int
VideoPipeline::start_frame (void)
{
  return m_start_frame;
}

int
VideoPipeline::end_frame (void)
{
  return m_end_frame;
}

void
VideoPipeline::set_start_frame (int v)
{
  m_start_frame = v;
}

void
VideoPipeline::set_end_frame (int v)
{
  m_end_frame = v;
}

float
VideoPipeline::progress (void)
{
  return m_processing_progress;
}

void
VideoPipeline::processing_thread (void)
{
  AlgoPipeline::ptr ap = AlgoPipeline::make (m_config);

  ap->create_all ();
  m_video_helper->go_to_frame (m_start_frame);

  const float to_process       = m_end_frame - m_start_frame + 1;
  int         frames_processed = 0;

  do
    {
      ap->process_frame (m_video_helper->image (),
                         m_video_helper->depth_map ());

      frames_processed++;
      m_processing_progress = frames_processed / to_process;
      progress_changed ();

      if (frames_processed == to_process)
        break;
    } while (m_run_processing && m_video_helper->next_frame ());
  set_running (false);
}
}