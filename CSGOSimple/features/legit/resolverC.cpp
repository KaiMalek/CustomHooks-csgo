#include "resolverC.h"
#include "../../valve_sdk\csgostructs.hpp"
#include "../../valve_sdk\sdk.hpp"
#include "../../helpers\utils.hpp"

float RandomNumber(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

int pResolver::aim_targ = 0;

namespace pResolver2
{
	pResolver::stored pResolver2::array_[65];
}
using namespace pResolver2;

bool ammolower(C_BasePlayer* pLocal, int guy)
{
	if (!pLocal)	return false;

	C_BaseCombatWeapon* entwep = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->m_hActiveWeapon());

	if (!entwep) return false;

	if (entwep->IsReloading())
		pResolver2::array_[guy].clip = -69;

	if (pResolver2::array_[guy].clip != entwep->m_iClip1())
	{
		pResolver2::array_[guy].clip = entwep->m_iClip1();
		return true;
	}
	return false;
}

void pResolver::Run() //disableda tm cuz its full of shit and too lazy to make better now becausel azy
{
	C_BasePlayer* pLocal = (C_BasePlayer*)g_LocalPlayer;
	if (!pLocal)
		return;

	if (!pLocal->IsAlive() || !pLocal->IsPlayer())
		return;

	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i)
	{
		//if(GetAsyncKeyState(VK_F4))
		//	pResolver2::array_[i].count = 0;

		auto entity = C_BasePlayer::GetPlayerByIndex(i);

		if (!entity || !entity->IsPlayer())
			continue;

		if (entity == pLocal || !entity->IsPlayer() || entity->IsDormant())
			continue;

		if (entity->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		if (i < 65)
		{

			if (!entity->IsAlive())
			{
				pResolver2::array_[i].count = 0;
				continue;
			}
			CCSGOPlayerAnimState* animstate = entity->getanimstate();

			if (animstate)
			{
				float timeStanding = animstate->m_flTimeSinceStoppedMoving;
				float velocity = (entity->m_vecVelocity().Length2D());
				float lby_ = entity->lby();
				pResolver2::array_[i].standTime = timeStanding;

				//	Utils::ConsolePrint("Velocity %f lby_ %f standtime %f oldlby %f \n", velocity, lby_, timeStanding, array_[i].oldLBY); // adding later MAYHBE
				if (timeStanding < 0.22f)
				{
					if (velocity < 3.f)
					{
						Utils::ConsolePrint("lby not updated (can crash maybe) (0.22) %f \n", timeStanding);

						float delta = array_[i].oldLBY - lby_;
						Utils::ClampYaw(delta);

						if (abs(delta) < 35.f)
						{
							pResolver2::array_[i].oldLBY = lby_;
							array_[i].yaw = lby_;
						}




					}
					else if (velocity > 35.f)
					{
						pResolver2::array_[i].wasMovingRecently = true;
						array_[i].yaw = lby_;
						pResolver2::array_[i].oldLBY = lby_;
						//		Utils::ConsolePrint("lby updating, moving %f \n", timeStanding);
					}

				}
				else if (pResolver2::array_[i].standTime < 5.f)
				{
					float delta = array_[i].oldLBY - lby_;
					Utils::ClampYaw(delta);

					if (abs(delta) > 35.f) {
						Utils::ConsolePrint("[Resolver] OLD lby_ %f | Current %f\n", pResolver2::array_[i].oldLBY, lby_);
						array_[i].yaw = array_[i].oldLBY;
					}
				}

			}
			else
				Utils::ConsolePrint("animstate broken update offset \n");

			if (pLocal->m_iShotsFired() <= 0 || !ammolower(pLocal, i)) //
			{
				//
				
				QAngle ang = entity->m_angEyeAngles();
				float lby_ = entity->m_flLowerBodyYawTarget();
				pResolver2::array_[i].res_state = Lowerbody;
				entity->m_angEyeAngles() = QAngle(ang.pitch, lby_, ang.roll);
				
				//		pResolver2::array_[i].count = 0;
				continue;
			}

			if (pResolver::aim_targ != i)
			{
				//	pResolver2::array_[i].count = 0;
				continue;
			}


			QAngle ang = entity->m_angEyeAngles();


			//	if (ang.pitch < 50 && ang.pitch > -50) //gay asf legit check XD
			//		continue;

			float eyeangles = ang.yaw;
			Utils::ClampYaw(eyeangles);

			float lby_ = entity->lby();

			float delta = eyeangles - lby_;
			Utils::ClampYaw(delta);






			bool bUsingFakeAngle = fabsf(delta) > 35.0f;
			bool gay = false;
			if (entity->m_vecVelocity().Length2D() > 35.f)
			{
				pResolver2::array_[i].res_state = Lowerbody;
				pResolver2::array_[i].yaw = lby_;
				entity->m_angEyeAngles() = QAngle(ang.pitch, lby_, ang.roll);
				gay = true;
			}
			else if (entity->m_vecVelocity().Length2D() > 5.f && entity->m_vecVelocity().Length2D() < 35.f)
			{

			}
			
			else
			{
				if (pResolver2::array_[i].method_use != -1)
				{

					if (pResolver2::array_[i].kk >= 1)
					{
						pResolver2::array_[i].kk = 0;
						pResolver2::array_[i].method_use = -1; // only use it 2x
						return;
					}
					float rand = RandomNumber(100.f, 180.f);
					rand = entity->m_flLowerBodyYawTarget() + rand;
					Utils::ClampYaw(rand);
					switch (pResolver2::array_[i].method_use)
					{

					case 0:
						entity->m_angEyeAngles() = QAngle(ang.pitch, lby_ + 110.f, ang.roll);
						pResolver2::array_[i].yaw = lby_;
						break;

					case 1:
						entity->m_angEyeAngles() = QAngle(ang.pitch, delta, ang.roll);
						pResolver2::array_[i].yaw = delta;
						break;

					case 2:
						pResolver2::array_[i].yaw = -lby_;
						entity->m_angEyeAngles() = QAngle(ang.pitch, -lby_, ang.roll);
						break;

					case 3:
						lby_ += lby_ / 2;
						Utils::ClampYaw(lby_);
						pResolver2::array_[i].yaw = lby_;
						entity->m_angEyeAngles() = QAngle(ang.pitch,lby_, ang.roll);
						break;

					case 4:

						pResolver2::array_[i].yaw = rand;
						entity->m_angEyeAngles() = QAngle(ang.pitch, rand, ang.roll);
						break;

					case 5:
						lby_ -= lby_ / 2;
						Utils::ClampYaw(lby_);
						pResolver2::array_[i].yaw = lby_;
						entity->m_angEyeAngles() = QAngle(ang.pitch, lby_, ang.roll);

						break;
					}

					pResolver2::array_[i].yaw = rand;
					entity->m_angEyeAngles() = QAngle(ang.pitch, rand, ang.roll);

					Utils::ConsolePrint("Using method_use custom with method %i\n", pResolver2::array_[i].method_use);
					pResolver2::array_[i].kk++;
				}
				else
				{

					
					//0 - lowerbody
					if (pResolver2::array_[i].count == 0)
					{
						pResolver2::array_[i].res_state = Lowerbody;
						if (lby_ == eyeangles)
						{

						}
						pResolver2::array_[i].yaw = lby_;
						entity->m_angEyeAngles() = QAngle(ang.pitch, lby_ , ang.roll);

					}
					//1 - delta
					else if (pResolver2::array_[i].count == 1)
					{
						pResolver2::array_[i].res_state = Delta;
						pResolver2::array_[i].yaw = delta;
						entity->m_angEyeAngles() = QAngle(ang.pitch, delta, ang.roll);


					}
					//2, 3  - inverse
					else if (pResolver2::array_[i].count <= 3 && pResolver2::array_[i].count > 1)
					{
						pResolver2::array_[i].res_state = Inverse;
						if (lby_ < -20.f || lby_ > 20.f)
						{
							pResolver2::array_[i].yaw = -lby_;
							entity->m_angEyeAngles() = QAngle(ang.pitch, -lby_, ang.roll);
						}

						else
						{
							float yaw = RandomNumber(-180.f, 180.f);
							Utils::ConsolePrint("%f \n", yaw);
							pResolver2::array_[i].yaw = yaw;
							entity->m_angEyeAngles() = QAngle(ang.pitch, yaw, ang.roll);
						}
						//	entity->m_angEyeAngles() = QAngle(ang.pitch, -lby + 45.f, ang.roll);
					}

					//4 - Half add
					else if (pResolver2::array_[i].count == 4)
					{
						pResolver2::array_[i].res_state = HalfAdd;
						if (lby_ > 40.f || lby_ < -40.f)
						{
							lby_ += lby_ / 2;
							Utils::ClampYaw(lby_);
							pResolver2::array_[i].yaw = lby_;
							entity->m_angEyeAngles() = QAngle(ang.pitch, lby_, ang.roll);
						}
						else
						{
							if (delta > 40.f || delta < -40.f)
							{
								delta += delta / 2;
								Utils::ClampYaw(delta);
								pResolver2::array_[i].yaw = delta;
								entity->m_angEyeAngles() = QAngle(ang.pitch, delta, ang.roll);
							}
							else
							{
								if (eyeangles > 40.f || eyeangles < -40.f)
								{
									eyeangles += eyeangles / 2;
									Utils::ClampYaw(eyeangles);
									pResolver2::array_[i].yaw = eyeangles;
									entity->m_angEyeAngles() = QAngle(ang.pitch, eyeangles, ang.roll);
								}
								else
								{
									float yaw = RandomNumber(-180.f, 180.f);
									Utils::ConsolePrint("%f \n", yaw);
									pResolver2::array_[i].yaw = yaw;
									entity->m_angEyeAngles() = QAngle(ang.pitch, yaw, ang.roll);
								}
							}
						}

					}
					//4 - Half minus
					else if (pResolver2::array_[i].count == 5)
					{
						pResolver2::array_[i].res_state = HalfMinus;
						if (lby_ > 40.f || lby_ < -40.f)
						{

							lby_ -= lby_ / 2;
							Utils::ClampYaw(lby_);
							pResolver2::array_[i].yaw = lby_;
							entity->m_angEyeAngles() = QAngle(ang.pitch, delta / 2, ang.roll);

						}
						else
						{
							if (delta > 40.f || delta < -40.f)
							{
								delta -= delta / 2;
								Utils::ClampYaw(delta);
								pResolver2::array_[i].yaw = delta;
								entity->m_angEyeAngles() = QAngle(ang.pitch, delta, ang.roll);
							}
							else
							{
								if (eyeangles > 40.f || eyeangles < -40.f)
								{
									eyeangles -= eyeangles / 2;
									Utils::ClampYaw(eyeangles);
									pResolver2::array_[i].yaw = eyeangles;
									entity->m_angEyeAngles() = QAngle(ang.pitch, eyeangles, ang.roll);
								}
								else
								{
									float yaw = RandomNumber(-180.f, 180.f);
									Utils::ConsolePrint("%f \n", yaw);
									pResolver2::array_[i].yaw = yaw;
									entity->m_angEyeAngles() = QAngle(ang.pitch, yaw, ang.roll);
								}
							}

						}
					}
					//etc random brute force 5 , 6 ,7 
					else if (pResolver2::array_[i].count > 5 && pResolver2::array_[i].count < 8)
					{
						pResolver2::array_[i].res_state = Bruteforce;
						float yaw = RandomNumber(-180.f, 180.f);
						Utils::ConsolePrint("%f \n", yaw);
						pResolver2::array_[i].yaw = yaw;
						entity->m_angEyeAngles() = QAngle(ang.pitch, yaw, ang.roll);
					}

					else if (pResolver2::array_[i].count >= 8)
					{
						pResolver2::array_[i].count = -1;
						pResolver2::array_[i].res_state = Lowerbody;
						pResolver2::array_[i].yaw = lby_;
						entity->m_angEyeAngles() = QAngle(ang.pitch, lby_, ang.roll);
					}

					// works but not consistant 
					// needs smthng here tho 

					pResolver2::array_[i].method_use = -1;
					pResolver2::array_[i].count++;
				}

			}
			

			if (!gay)
			{
				float rand = RandomNumber(-100.f, 180.f);
				rand = entity->m_flLowerBodyYawTarget() + rand;
				Utils::ClampYaw(rand);


				pResolver2::array_[i].yaw = rand;
				entity->m_angEyeAngles() = QAngle(ang.pitch, rand, ang.roll);
			}


			Utils::ConsolePrint("Velocity: %f Delta: %f  fake: %f  lby_: %f Res: %i method: %i target: %i\n", entity->m_vecVelocity().Length2D(), delta, eyeangles, lby_, (int)pResolver2::array_[i].res_state, pResolver2::array_[i].method_use, i);



		}
	}
}