#include "render_framework.hpp"
#include "../ImGui/imgui.h"

void renderer::draw_text(int x, int y, color clr, std::string str) {
	ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImColor(clr.r, clr.g, clr.b, clr.a), str.c_str());
}

int renderer::text_width(std::string str) {
	int width = ImGui::CalcTextSize(str.c_str()).x;
	return width;
}

int renderer::text_height(std::string str) {
	int height = ImGui::CalcTextSize(str.c_str()).y;
	return height;
}

void renderer::draw_rect(int x, int y, int w, int h, color clr, float rounding) {
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(clr.r, clr.g, clr.b, clr.a), rounding);
}

void renderer::draw_filled_rect(int x, int y, int w, int h, color clr, float rounding) {
	ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(clr.r, clr.g, clr.b, clr.a), rounding);
}

void renderer::rect_fade(int x, int y, int w, int h, color clr_top_left, color clr_top_right, color clr_bottom_right, color clr_bottom_left, float rounding) {
	ImGui::GetOverlayDrawList()->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + w, y + h), ImColor(clr_top_left.r, clr_top_left.g, clr_top_left.b, clr_top_left.a), ImColor(clr_top_right.r, clr_top_right.g, clr_top_right.b, clr_top_right.a), ImColor(clr_bottom_right.r, clr_bottom_right.g, clr_bottom_right.b, clr_bottom_right.a), ImColor(clr_bottom_left.r, clr_bottom_left.g, clr_bottom_left.b, clr_bottom_left.a));
}

void renderer::draw_line(int x, int y, int to_x, int to_y, int width, color clr) {
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(x, y), ImVec2(to_x, to_y), ImColor(clr.r, clr.g, clr.b), width);
}

void renderer::draw_triangle(int x, int y, int top_x, int top_y, int bottom_x, int bottom_y, color clr, int thickness) {
	ImGui::GetOverlayDrawList()->AddTriangle(ImVec2(x, y), ImVec2(top_x, top_y), ImVec2(bottom_x, bottom_y), ImColor(clr.r, clr.g, clr.b, clr.a), thickness);
}

void renderer::draw_triangle_filled(int x, int y, int top_x, int top_y, int bottom_x, int bottom_y, color clr) {
	ImGui::GetOverlayDrawList()->AddTriangleFilled(ImVec2(x, y), ImVec2(top_x, top_y), ImVec2(bottom_x, bottom_y), ImColor(clr.r, clr.g, clr.b, clr.a));
}

void renderer::hsv_to_rgb(float H, float S, float V, color &clr_to_change) {
    float s = S / 100;
    float v = V / 100;
    float C = s * v;
    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    float m = v - C;
    float r, g, b;
    if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    }
    else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    }
    else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    }
    else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    }
    else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    }
    else {
        r = C, g = 0, b = X;
    }
    int R = (r + m) * 255;
    int G = (g + m) * 255;
    int B = (b + m) * 255;
    int r_h = r * 255;
    int g_h = g * 255;
    int b_h = b * 255;
    clr_to_change.r = R;
    clr_to_change.g = G;
    clr_to_change.b = B;
}


void renderer::push_clip_rect(int x, int y, int w, int h, bool intersect) {
    ImGui::GetOverlayDrawList()->PushClipRect(ImVec2(x, y), ImVec2(x + w, y + h), intersect);
}

void renderer::pop_clip_rect() {
    ImGui::GetOverlayDrawList()->PopClipRect();
}





