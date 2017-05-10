#pragma once

#include "common.h"

HMODULE dll = NULL;
bool connected[] = { false, false };
#define MAX_FNUM 10
#define MAX_HNUM 2
//sharpnow::Vector3 finger_top[MAX_FNUM], finger_dir[MAX_FNUM];
//list<sharpnow::Vector3> trajectory[MAX_FNUM];
int traj_cnt[MAX_FNUM];
int finger_num = 0, hand_num = 0;
//sharpnow::Vector3 grip_pos={.0,.0,.0};
//sharpnow::Vector3 base_pos={.0,.0,.0};
//sharpnow::Vector3 finger_con_pos={.0,.0,.0};
//sharpnow::Vector3 finger_con_pos_base={.0,.0,.0};
int ges = 0;
float rx, ry = 0.0;
float rx_base, ry_base = 0.0;
bool grip_b, rx_b, ry_b = false;
bool finger_contact = false;
bool finger_contact_b = false;
float HandBallRadius = 0.0;
cv::Point3d fingersPos[5], fingerBone[5][3], handsPos[2], handsPosBase[2];
bool hand_base = false;
int numFingers = 0;
bool grabbed = false;
float yaw_angle = 0.0;
int RorL = 0;
bool touch_right, touch_right_base = false;
bool pinch, pinch_base = false;
bool grab, grab_base = false;
bool zoom, zoom_base = false;
float zoomscale = 1.0;
cv::Point3d finger_touch_base, finger_touch;
cv::Point3d finger_pinch_base, finger_pinch;
cv::Point3d finger_grab_base, finger_grab;
int init_display = 0;
#define ModelNum 10
float xpos[ModelNum], ypos[ModelNum], zpos[ModelNum] = { 0 };
float _xpos[ModelNum], _ypos[ModelNum], _zpos[ModelNum] = { 0 };
float xrot[ModelNum], yrot[ModelNum];
float _xrot[ModelNum], _yrot[ModelNum];
float _mscale[ModelNum], mscale[ModelNum];
bool index_tap, middle_tap = false;
int pinch_obj_idx = -1; int grab_obj_idx = -1;
bool grab_key = false;
float finger1_LR_dis = 1; float finger1_LR_dis_base = 1;

bool rot_byLP = false, rot_base = false;
cv::Point3d finger_rot_base, finger_rot;


int img_w = 800; int img_h = 800;

cv::Mat img_lp(img_h, img_w, CV_8UC3);

cv::Point3d coordinate_trans(Leap::Vector a)
{
	cv::Point3d b;
	float theta = 60.0;
	b.x = a.x;
	b.y = a.y * sin(theta * 3.1415 / 180) + a.z * cos(theta * 3.1415 / 180);
	b.z = a.y * cos(theta * 3.1415 / 180) - a.z * sin(theta * 3.1415 / 180) - 100;

	/*b.x=a.x;
	b.y=a.y;
	b.z=a.z;*/
	return b;
}

class SampleListener : public Leap::Listener
{
public:
	virtual void onInit(const Leap::Controller&);
	virtual void onConnect(const Leap::Controller&);
	virtual void onDisconnect(const Leap::Controller&);
	virtual void onExit(const Leap::Controller&);
	virtual void onFrame(const Leap::Controller&);
private:
};

const std::string fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };
const std::string stateNames[] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };
//
void SampleListener::onInit(const Leap::Controller& controller)
{
	std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Leap::Controller& controller)
{
	std::cout << "Connected" << std::endl;

	controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);


	controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
	controller.config().setFloat("Gesture.KeyTap.MinDownVelocity", 40.0);
	controller.config().setFloat("Gesture.KeyTap.HistorySeconds", .2);
	controller.config().setFloat("Gesture.KeyTap.MinDistance", 8.0);
	controller.config().save();

	controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);

	controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Leap::Controller& controller)
{
	// Note: not dispatched when running in a debugger.
	std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Leap::Controller& controller)
{
	std::cout << "Exited" << std::endl;
}

void drawHands(const Leap::Frame &frame)
{
	float scale = 1.0;
	Leap::HandList hands = frame.hands();
	numFingers = 0;
	cv::Point3d finger1_L, finger1_R;
	//finger1_L.y=1000; finger1_R.y=1000;
	hand_num = 0;
	for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl)
	{
		// Get the first hand
		const Leap::Hand hand = *hl;
		//printf(" %f   %f   %f\r",hand.grabStrength(),hand.pinchStrength(),hand.confidence());
		hand_num++;
	}
	//	cout<<hand_num<<endl;
	if (hand_num == 1)
	{
		for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl)
		{
			// Get the first hand
			const Leap::Hand hand = *hl;
			//printf(" %f   %f   %f\r",hand.grabStrength(),hand.pinchStrength(),hand.confidence());
			RorL = hand.isLeft() ? 1 : 0;
			cv::Point3d handPos3d = coordinate_trans(hand.palmPosition()/*.stabilizedPalmPosition()*/);
			cv::Point handPos(img_w / 2 + handPos3d.x*scale, img_h / 2 - handPos3d.z *scale);
			//cv::Point3d handPos3d(hand.palmPosition().x*scale , hand.palmPosition().y*scale , hand.palmPosition().z*scale);
			handsPos[RorL] = handPos3d;
			const Leap::FingerList fingers = hand.fingers();
			numFingers = 0;
			int edge = 10;
			for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl)
			{
				const Leap::Finger finger = *fl;

				Leap::Vector fingerVec = finger.stabilizedTipPosition();//.tipPosition();
																		//Leap::Vector fingerVec = finger.tipPosition();
				cv::Point fingerPos(img_w / 2 + fingerVec.x*scale, img_h / 2 + fingerVec.z*scale);
				//cv::Point3d fingerPos3d(fingerVec.x*scale  ,fingerVec.y*scale  , fingerVec.z*scale  );

				fingerVec.x += fingerVec.y / 5; /////////////////У��  ��Ҫ///////////////////////// 

				cv::Point3d fingerPos3d = coordinate_trans(fingerVec);
				fingersPos[numFingers] = fingerPos3d;

				if (finger.type() == 1)
				{
					if (fingerPos3d.y <60)	rot_byLP = true;
					else
						rot_byLP = false;
					pinch_obj_idx = 0;
					if (rot_byLP)
					{
						finger_rot = fingerPos3d;
						if (!rot_base)
						{
							finger_rot_base = fingerPos3d;	rot_base = true;
						}
					}
					else
					{
						rot_base = false;
					}
					cv::putText(img_lp, NumToString(finger_rot.x) + "  " + NumToString(finger_rot.y) + "  " + NumToString(finger_rot.z), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
					cv::putText(img_lp, NumToString(finger_rot_base.x) + "  " + NumToString(finger_rot_base.y) + "  " + NumToString(finger_rot_base.z), cv::Point(50, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
					//cv::putText(img_lp, NumToString(_xrot[current_move]) + "  " + NumToString(xrot[current_move]), cv::Point(50, 200), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
				}
				for (int b = 0; b < 4; ++b)
				{
					Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
					Leap::Bone bone = finger.bone(boneType);
					cv::Point3d bonePos_p3d = coordinate_trans(bone.prevJoint());
					cv::Point bonePos_p(img_w / 2 + bonePos_p3d.x*scale, img_h / 2 - bonePos_p3d.z*scale);
					cv::Point3d bonePos_n3d = coordinate_trans(bone.nextJoint());
					cv::Point bonePos_n(img_w / 2 + bonePos_n3d.x*scale, img_h / 2 - bonePos_n3d.z*scale);
					cv::circle(img_lp, bonePos_p, 10, cv::Scalar(0, 255, 0), 3, 8, 0);
					cv::line(img_lp, bonePos_p, bonePos_n, cv::Scalar(255, 255, 255), 3, 8, 0);
					cv::circle(img_lp, bonePos_n, 10, cv::Scalar(0, 255, 0), 3, 8, 0);
				}
				numFingers++;
			}
			if (rot_byLP)
				cv::circle(img_lp, handPos, 15, cv::Scalar(0, 0, 255), 3, 8, 0);
			else
				cv::circle(img_lp, handPos, 15, cv::Scalar(0, 255, 0), 3, 8, 0);
		}
	}

	if (hand_num == 2)
	{
		for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl)
		{
			const Leap::Hand hand = *hl;
			RorL = hand.isLeft() ? 1 : 0;
			cv::Point3d handPos3d = coordinate_trans(hand.palmPosition()/*.stabilizedPalmPosition()*/);
			cv::Point handPos(img_w / 2 + handPos3d.x*scale, img_h / 2 - handPos3d.z *scale);

			const Leap::FingerList fingers = hand.fingers();
			numFingers = 0;
			int edge = 20;
			for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl)
			{
				const Leap::Finger finger = *fl;

				Leap::Vector fingerVec = finger.stabilizedTipPosition();//.tipPosition();
																		//Leap::Vector fingerVec = finger.tipPosition();
				cv::Point fingerPos(img_w / 2 + fingerVec.x*scale, img_h / 2 + fingerVec.z*scale);
				//cv::Point3d fingerPos3d(fingerVec.x*scale  ,fingerVec.y*scale  , fingerVec.z*scale  );

				fingerVec.x += fingerVec.y / 5; /////////////////У��  ��Ҫ///////////////////////// 

				cv::Point3d fingerPos3d = coordinate_trans(fingerVec);
				fingersPos[numFingers] = fingerPos3d;

				if (RorL == 1)	finger1_L = fingersPos[1];
				if (RorL == 0)	finger1_R = fingersPos[1];
				//cv::putText(img_lp,NumToString(_xrot[current_move]) +"  " +NumToString(xrot[current_move]) ,cv::Point(50,200),cv::FONT_HERSHEY_SIMPLEX, 1.0,cv::Scalar(255,255,255),1);


				for (int b = 0; b < 4; ++b)
				{
					Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
					Leap::Bone bone = finger.bone(boneType);
					cv::Point3d bonePos_p3d = coordinate_trans(bone.prevJoint());
					cv::Point bonePos_p(img_w / 2 + bonePos_p3d.x*scale, img_h / 2 - bonePos_p3d.z*scale);
					cv::Point3d bonePos_n3d = coordinate_trans(bone.nextJoint());
					cv::Point bonePos_n(img_w / 2 + bonePos_n3d.x*scale, img_h / 2 - bonePos_n3d.z*scale);
					cv::circle(img_lp, bonePos_p, 10, cv::Scalar(0, 255, 0), 3, 8, 0);
					cv::line(img_lp, bonePos_p, bonePos_n, cv::Scalar(255, 255, 255), 3, 8, 0);
					cv::circle(img_lp, bonePos_n, 10, cv::Scalar(0, 255, 0), 3, 8, 0);
				}
				if (zoom)
					cv::circle(img_lp, handPos, 15, cv::Scalar(0, 0, 255), 3, 8, 0);
				else
					cv::circle(img_lp, handPos, 15, cv::Scalar(0, 255, 0), 3, 8, 0);
				numFingers++;
			}
		}

		pinch_obj_idx = 0;

		cv::Point3d finger1_L_base, finger1_R_base;



		cv::putText(img_lp, NumToString(finger1_LR_dis) + "  " + NumToString(finger1_LR_dis_base), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		cv::putText(img_lp, NumToString(finger1_L.x) + "  " + NumToString(finger1_L.y) + "  " + NumToString(finger1_L.z), cv::Point(50, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		cv::putText(img_lp, NumToString(finger1_R.x) + "  " + NumToString(finger1_R.y) + "  " + NumToString(finger1_R.z), cv::Point(50, 150), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		cv::putText(img_lp, NumToString(zoomscale), cv::Point(50, 250), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		//printf("%d \r",current_model);
	}
	//else
	//{
	//	finger1_LR_dis=1;
	//	finger1_LR_dis_base=1;
	//}

	cv::imshow("finger", img_lp);
	if (cv::waitKey(1) == 27)
		exit(0);
	img_lp.setTo(0);
}
void SampleListener::onFrame(const Leap::Controller& controller)
{
	const Leap::Frame frame = controller.frame();
	Leap::CircleGesture circle;
	Leap::SwipeGesture swipe;


	// Get the most recent frame and report some basic information
	//drawHands(frame);

	Leap::GestureList gestures = frame.gestures();
	Leap::Vector swipeDirection;
	for (Leap::GestureList::const_iterator gl = gestures.begin(); gl != frame.gestures().end(); gl++)
	{
		switch ((*gl).type()) {
		case Leap::Gesture::TYPE_CIRCLE:
			//Handle circle gestures
			circle = (*gl);
			if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI / 2) {
				if(m_model_scale < 10)
					m_model_scale *= 1.03;

			}
			else
			{
				if(m_model_scale > 1)
					m_model_scale /= 1.03;
			}

			break;
		case Leap::Gesture::TYPE_KEY_TAP:
			//Handle key tap gestures
			//autoRotate = !autoRotate;
			break;
		case Leap::Gesture::TYPE_SCREEN_TAP:
			//Handle screen tap gestures
			m_model_scale = 1;
			break;
		case Leap::Gesture::TYPE_SWIPE:
			//Handle swipe gestures
			swipe = (*gl);
			
			swipeDirection = swipe.direction().normalized();
			for (int i = 0; i < 10; i++) {
				m_rotate_vec += (float)(10 - i) * glm::vec3(swipeDirection.x,
					swipeDirection.y,
					swipeDirection.z) ;

			}
			

			break;
		default:
			//Handle unrecognized gestures
			break;
		}
	}
/////////////////////////////////crack
	for (Leap::HandList::const_iterator hl = frame.hands().begin(); hl != frame.hands().end(); hl++) {
		
			//if((*hl).grabStrength() > 0.8) {
			//	Leap::Vector pos = (*hl).palmPosition().normalized();
			//	m_translate_vec = glm::vec3(pos.x, pos.y, pos.z);
			//}
		

	}
////////////////////////////////

/*
GESTURE: Drag
ys
2017/5/10
grab and move

TO-DO: stability need to improve
*/

	Leap::Hand rightHand = frame.hands().rightmost();
	Leap::Vector rHandVelocity = rightHand.palmVelocity().normalized();
	if (rightHand.grabStrength() > 0.7) {
		m_translate_vec += glm::vec3(rHandVelocity.x,
			rHandVelocity.y,
			rHandVelocity.z)*(float)5;
			
	}

	Sleep(30);

}

void RunLP()
{
	// Create a sample listener and controller
	SampleListener listener;
	Leap::Controller controller;
	// Have the sample listener receive events from the controller
	controller.addListener(listener);
	controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
	controller.setPolicy(Leap::Controller::POLICY_IMAGES);
	while (1)
	{
	}
}