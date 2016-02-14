#include "TowerTracker.h"

int main()
{
	TowerTracker::ThresholdValues thresh = {75,90,175,255,240,255};
	TowerTracker tracker (thresh);
	tracker.run();

	/*
	int key;
	while (true)
	{
		key = waitKey(200) & 255;
		if (key == 27) {
			tracker.Stop();
			break;
		}
	}
	*/
}
