// griefer, block bot, chat spam, name changer and more stuff wheni figure out sadge :(

#include "griefer.hpp"
#include "../../helpers/math.hpp"
#include "../../options.hpp"

CBlockBot* g_BlockBot = new CBlockBot();

float yaw3(Vector v) {
    auto x = v[0];
    auto y = v[1];
    auto z = v[2];

    return (float)(atan2(y, x) * 180 / M_PI);
};

float dist3(Vector a, Vector b) {
    auto ax = a[0];
    auto ay = a[1];
    auto az = a[2];
    auto bx = b[0];
    auto by = b[1];
    auto bz = b[2];

    auto dx = ax - bx;
    auto dy = ay - by;
    auto dz = az - bz;

    return sqrt(dx * dx + dy * dy + dz * dz);
};

Vector sub3(Vector a, Vector b) {
    auto ax = a[0];
    auto ay = a[1];
    auto az = a[2];
    auto bx = b[0];
    auto by = b[1];
    auto bz = b[2];

    return Vector(ax - bx, ay - by, az - bz);
};

Vector add3(Vector a, Vector b) {
    auto ax = a[0];
    auto ay = a[1];
    auto az = a[2];
    auto bx = b[0];
    auto by = b[1];
    auto bz = b[2];

    return Vector(ax + bx, ay + by, az + bz);
}

float len3(Vector v) {
    return dist3(v, Vector(0, 0, 0));
};


void CBlockBot::draw() {
    if (Target && !Target->IsAlive())
        Target = nullptr;

    for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
        auto entity = C_BasePlayer::GetPlayerByIndex(i);
        if (!entity) continue;
        if (!entity->IsPlayer()) continue;
        if (entity == g_LocalPlayer) continue;
        if (!entity->IsAlive()) continue;

        if (!Target || Target == nullptr)
            Target = entity;

        auto playerOrigin = g_LocalPlayer->m_vecOrigin();
        auto entityOrigin = entity->m_vecOrigin();
        auto targetOrigin = Target->m_vecOrigin();
        if (dist3(playerOrigin, targetOrigin) > dist3(playerOrigin, entityOrigin)) {
            Target = entity;
        }
    }
}

void CBlockBot::cmove(CUserCmd* pCmd) {
    if (Target && Target->IsAlive()) {
        QAngle LocalAngles;
        g_EngineClient->GetViewAngles(&LocalAngles);
        auto VecForward = sub3(Target->GetRenderOrigin(), g_LocalPlayer->GetRenderOrigin());
        auto otherYaw = yaw3(VecForward);
        auto TargetSpeed = len3(Target->m_vecVelocity());
        Vector enemyPos = Target->m_vecOrigin();
        float dist = g_LocalPlayer->m_angAbsOrigin().DistTo(Target->m_angAbsOrigin());

        if (dist < 60 && enemyPos.z + 10 < g_LocalPlayer->m_vecOrigin().z) {
            float yaw = pCmd->viewangles.yaw;
            Vector offset = g_LocalPlayer->m_vecOrigin() - enemyPos;
            Vector translatedVelocity = Vector(
                (float)(offset.x * cos(yaw / 180 * PI_F) + offset.y * sin(yaw / 180 * PI_F)),
                (float)(offset.y * cos(yaw / 180 * PI_F) - offset.x * sin(yaw / 180 * PI_F)),
                offset.z
            );

            pCmd->sidemove = translatedVelocity.y * 100;
            pCmd->forwardmove = -translatedVelocity.x * 100;
            pCmd->upmove = translatedVelocity.z;
        }
        else {
            auto cmdMove = Vector(0, 0, 0);
            auto DiffYaw = otherYaw - LocalAngles[1];

            if (DiffYaw > 180)
                DiffYaw = DiffYaw - 360;
            else if (DiffYaw < -180)
                DiffYaw = DiffYaw + 360;

            if (TargetSpeed > 285)
                cmdMove[0] = -abs(TargetSpeed);

            if (DiffYaw > 0.25)
                cmdMove[1] = -450;
            else if (DiffYaw < -0.25)
                cmdMove[1] = 450;

            pCmd->forwardmove = cmdMove[0];
            pCmd->sidemove = cmdMove[1];
            pCmd->upmove = 0;
        }
    }

}