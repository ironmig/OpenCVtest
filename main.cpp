#include "TowerTracker.h"

int main()
{
	TowerTracker::ThresholdValues thresh = {77.5,87.5,204,242.25,242.25,255};
	TowerTracker tracker (thresh);
	tracker.run();
}
