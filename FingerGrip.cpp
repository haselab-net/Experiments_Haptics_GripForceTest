#include "FingerGrip.h"

void Finger::AddForce(double f) {
	force += f;
}
void Finger::Step(double dt) {
	length += force * dt / mass;
}

class FingerGrip {
	Posed pose;
	std::vector<Finger> fingers;
	void Step(Posed p, double dt) {
		pose = p;
		for (auto finger: fingers) {
			finger.Step(dt);
		}
	}
};