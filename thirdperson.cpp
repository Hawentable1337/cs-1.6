#include "client.h"

void ThirdPerson(struct ref_params_s* pparams)
{
	if (cvar.visual_chase_cam && bAliveLocal() && DrawVisuals && (!cvar.route_auto || cvar.route_draw_visual) && GetTickCount() - HudRedraw <= 100)
	{
		Vector r, u, b;
		VectorMul(pparams->right, 0, r);
		VectorMul(pparams->up, cvar.visual_chase_up, u);
		VectorMul(pparams->forward, -cvar.visual_chase_back, b);

		Vector tracestart = pparams->vieworg;
		pmtrace_t trace;
		g_Engine.pEventAPI->EV_SetTraceHull(2);
		g_Engine.pEventAPI->EV_PlayerTrace(tracestart, tracestart + r + u + b, PM_GLASS_IGNORE | PM_STUDIO_BOX, -1, &trace);
		
		VectorCopy(tracestart + r + u + b, pparams->vieworg);
		if (trace.fraction != 1.0f) VectorCopy(trace.endpos, pparams->vieworg);
	}
}
