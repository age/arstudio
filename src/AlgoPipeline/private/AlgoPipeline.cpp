#include <AlgoPipeline.hpp>

/**
 *      The constructor. During build, CMake inserts code specific to each
 * algorithm
 *      being built.
 *
 *      \param config is passed to every algorithm being instantified
 */

AlgoPipeline::AlgoPipeline (Config::ptr config)
{
	@ALGO_CODE@
}

/**
 *      Convenience function to initialize all algorithms, we do not want
 *      to do that in constructor.
 */

void
AlgoPipeline::create_all (void)
{
	for (auto algo : algo_list)
		{
			if (!algo->create ())
				{
					throw std::runtime_error ("failed to create(): " +
						algo->id_string ());
				}
		}
}

/**
 *      The destructor, in turn calls destructors for every algorithm.
 */

AlgoPipeline::~AlgoPipeline ()
{
	for (auto algo : algo_list)
		{
			delete algo;
		}
}

/**
 *      Convenience function. Creates an instance of AlgoPipeline wrapped
 * in boost::shared_ptr.
 *
 *      \param config see constructor
 */

AlgoPipeline::ptr
AlgoPipeline::make (Config::ptr config)
{
	return std::make_shared<AlgoPipeline> (config);
}

/**
 *      Processes a single frame of input with every algorithm.
 *
 *      \param image RGB image to be passed to algorithms
 *      \param dmap depth map to be passed to algorithms
 */

void
AlgoPipeline::process_frame (const cv::Mat & image, const cv::Mat & dmap)
{
	for (auto algo : algo_list)
		{
			algo->run (image, dmap);
		}
	Logger::getInstance ().advanceFrame ();
}
