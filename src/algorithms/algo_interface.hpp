#ifndef ALGO_INTERFACE_HPP
#define ALGO_INTERFACE_HPP

#include <opencv2/highgui/highgui.hpp>
#include <config.hpp>

class IAbstractAlgorithm
{
public:
	virtual bool create () = 0;
	virtual bool run (cv::Mat &, cv::Mat &) = 0;

	IAbstractAlgorithm (Config * config)
	{
		this->config = config;
	};

	~IAbstractAlgorithm ()
	{
		delete config;
	}

protected:
	Config * config;
};
#endif // ALGO_INTERFACE_HPP
