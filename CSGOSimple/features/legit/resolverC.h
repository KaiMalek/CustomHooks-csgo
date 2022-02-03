#pragma once


class pResolver
{
public:

	enum ResolverState
	{
		Lowerbody,
		Delta,
		Inverse,
		HalfAdd,
		Bruteforce,
		HalfMinus
	};
	struct stored
	{
		int count;
		int kk;
		bool aimbot_target;
		float yaw = 0.f;
		int clip;
		int method_use;
		ResolverState res_state;
		bool wasMovingRecently;
		float standTime;
		float oldLBY;
		bool set;
	};

	static int aim_targ;
	static void Run();

};
namespace pResolver2
{
	extern pResolver::stored array_[65];
}