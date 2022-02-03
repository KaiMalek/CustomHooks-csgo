#include <string>
#include "../colorr.hpp"


namespace renderer {
		void draw_text(int x, int y, color clr, std::string str);
		int text_width(std::string str);
		int text_height(std::string str);
		void draw_rect(int x, int y, int w, int h, color clr, float rounding);
		void draw_filled_rect(int x, int y, int w, int h, color clr, float rounding);
		void rect_fade(int x, int y, int w, int h, color clr_top_left, color clr_top_right, color clr_bottom_right, color clr_bottom_left, float rounding);
		void draw_line(int x, int y, int to_x, int to_y, int width, color clr);
		void draw_triangle(int x, int y, int top_x, int top_y, int bottom_x, int bottom_y, color clr, int thickness);
		void draw_triangle_filled(int x, int y, int top_x, int top_y, int bottom_x, int bottom_y, color clr);
		void hsv_to_rgb(float H, float S, float V, color &clr_to_change);

		void push_clip_rect(int x, int y, int w, int h, bool intersect);
		void pop_clip_rect();
}