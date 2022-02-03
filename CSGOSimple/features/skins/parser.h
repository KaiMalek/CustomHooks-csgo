#pragma once

#include <cstdio>
#include <vector>
#include <memory>
#include <string>
#include <deque>

namespace valve_parser
{
	class document;
	class key_value;
	class object;

	enum encoding
	{
		utf8,
		utf16_le,
		utf16_be,
		utf32_le,
		utf32_be,
		max
	};

	enum
	{
		STRING = '"',
		OBJECT_OPEN = '{',
		OBJECT_CLOSE = '}',
		COMMENT = '/',
	};


	class str
	{
	public:
		char32_t* _start = nullptr;
		char32_t* _end = nullptr;
	public:
		std::string to_string() const
		{
			if (!_start || !_end)
				return "";

			//convert utf32 to utf16
			std::vector<char16_t> utf16;
			utf16.reserve(200);

			for (auto text = _start; text != _end + 1; text++)
			{
				if ((*text >= 0 && *text <= 0xD7FF) ||
					*text >= 0xE000 && *text <= 0xFFFF)
				{
					utf16.push_back(*text);
				}
				if (*text >= 0x10000 && *text <= 0x10FFFF)
				{
					const char32_t offset = *text - 0x10000;
					char16_t hi = (offset & 0xFFC00) >> 10;
					char16_t lo = offset & 0x3FF;
					hi += 0xD800;
					lo += 0xDC00;
					utf16.push_back(hi);
					utf16.push_back(lo);
				}
			}
			return std::string(utf16.begin(), utf16.end());
		}

		//warning: no exception handling
		int to_int() const
		{
			return std::stoi(to_string());
		}

		//checks whether the next read character equals expectedTag, if not 0 is returned
		static char32_t* parse_text_expected_tag(char32_t* p, const char32_t expected_tag, const bool ignore_space_tab_lf_cr)
		{
			for (; *p; p++)
			{
				//skip comment line
				if (*p == COMMENT && *(p + 1) && *(p + 1) == COMMENT)
				{
					for (; *p && *p != '\n';) p++;
				}

				if (*p == '[')
				{
					for (; *p && *p != ']';) p++;
					p++;
				}

				if (ignore_space_tab_lf_cr)
				{
					if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
						continue;
				}

				if (*p == expected_tag)
					return p;
				else
					return nullptr;
			}

			return nullptr;
		}

		static char32_t* end_reached(char32_t* p, char32_t expectedTag)
		{
			for (; *p; p++)
			{
				//skip comment line
				if (*p == COMMENT && *(p + 1) && *(p + 1) == COMMENT)
				{
					for (; *p && *p != '\n'; p++);
				}

				if (*p == '[')
				{
					for (; *p && *p != ']';) p++;
					p++;
				}

				if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
					continue;

				if (*p == expectedTag)
					return p;
				else
					return reinterpret_cast<char32_t*>(-1);
			}

			return nullptr;
		}

		static char32_t* parse_text_end_tag(char32_t* p, char32_t endTag)
		{
			for (; *p; p++)
			{
				//skip escape sequence
				//there are also hex escapes
				//\xAA45, no need to handle them tho
				if (*p == '\\')
					p += 2;

				if (*p == endTag)
					return p;
			}

			return nullptr;
		}
	};

	class util
	{
	public:
		static bool str_equ(const str str1, char* str2)
		{
			return str1.to_string() == std::string(str2);
		}

		static bool str_equ_i(const str str1, char* str2)
		{
			const auto str = str1.to_string();
			return str.compare(str2) == std::string::npos;
		}
	};

	class node
	{
	protected:
		document* _doc;
	public:
		std::vector<std::shared_ptr<node>> children;
		virtual bool parse();

		explicit node(document* doc)
		{
			_doc = doc;
		}

		virtual object* to_object()
		{
			return nullptr;
		}

		virtual key_value* to_key_value()
		{
			return nullptr;
		}
	};

	class key_value : public node
	{
	public:
		str key;
		str value;

		explicit key_value(document* doc) : node(doc) {}

		key_value* to_key_value() override
		{
			return this;
		}
	};

	class object : public node
	{
	public:
		str name;
		explicit object(document* doc);
		object* to_object() override;
		bool parse() override;
		std::shared_ptr<node> get_key_by_name(char* name);
	};

	class document
	{
	private:
		friend class object;
		friend class node;

		struct bom
		{
			char* str;
			size_t len;
		};
		bom boms_[encoding::max];

		char32_t* utf32_text_;
		char32_t* p_ = nullptr;
	public:
		std::shared_ptr<node> root;

		document()
		{
			boms_[utf8] = { "\xEF\xBB\xBF", 3 };
			boms_[utf16_le] = { "\xFF\xFE", 2 };
			boms_[utf16_be] = { "\xFE\xFF", 2 };
			boms_[utf32_le] = { "\xFF\xFE\x00\x00", 4 };
			boms_[utf32_be] = { "\x00\x00\xFE\xFF", 4 };
			utf32_text_ = nullptr;
		}

		~document()
		{
			delete[] utf32_text_;
		}

		std::shared_ptr<node> breadth_first_search(char* name, bool caseInsensitive = false) const
		{
			std::deque<std::shared_ptr<node>> q;
			q.push_back(root);
			while (!q.empty())
			{
				auto f = q.front();
				q.pop_front();

				if (f->to_key_value())
				{
					if (!caseInsensitive && util::str_equ(f->to_key_value()->key, name))
						return f;

					if (caseInsensitive && util::str_equ_i(f->to_key_value()->key, name))
						return f;
				}
				if (f->to_object())
				{
					if (!caseInsensitive && util::str_equ(f->to_object()->name, name))
						return f;

					if (caseInsensitive && util::str_equ_i(f->to_object()->name, name))
						return f;
				}

				for (auto& child : f->children)
					q.push_back(child);
			}

			return nullptr;
		}

		std::vector<std::shared_ptr<node>> breadth_first_search_multiple(char* name, bool caseInsensitive = false) const
		{
			std::vector<std::shared_ptr<node>> vec;
			std::deque<std::shared_ptr<node>> q;
			q.push_back(root);
			while (!q.empty())
			{
				auto f = q.front();
				q.pop_front();

				if (f->to_key_value())
				{
					if (!caseInsensitive && util::str_equ(f->to_key_value()->key, name))
						vec.push_back(f);

					if (caseInsensitive && util::str_equ_i(f->to_key_value()->key, name))
						vec.push_back(f);
				}
				if (f->to_object())
				{
					if (!caseInsensitive && util::str_equ(f->to_object()->name, name))
						vec.push_back(f);

					if (caseInsensitive && util::str_equ_i(f->to_object()->name, name))
						vec.push_back(f);
				}

				for (auto& child : f->children)
					q.push_back(child);
			}

			return vec;
		}

		//returns true when format is correct
		//parse from file
		bool load(char* path, encoding encoding)
		{
			FILE* f;
			fopen_s(&f, path, "rb");
			if (!f)
				return false;

			fseek(f, 0, SEEK_END);
			const auto size = ftell(f);
			if (!size)
				return false;

			size_t null_terminator_bytes = 0;
			if (encoding == encoding::utf16_be ||
				encoding == encoding::utf16_le)
				null_terminator_bytes = 2;

			if (encoding == encoding::utf8)
				null_terminator_bytes = 1;

			if (encoding == encoding::utf32_be ||
				encoding == encoding::utf32_le)
				null_terminator_bytes = 4;

			const auto text = new char[size + null_terminator_bytes];
			fseek(f, 0, SEEK_SET);
			if (fread(text, 1, size, f) != size)
				return false;

			for (size_t i = 0; i < null_terminator_bytes; i++)
				text[size + i] = 0;

			fclose(f);

			//convert to utf32
			//which makes everything easy to handle
			//but increases buffer size
			cnv_to_utf32(text, encoding);
			delete[] text;

			p_ = trim_space(utf32_text_);
			if (!p_)
				return false;

			return parse_deep();
		}
	private:
		char* skip_bom(char* p, const encoding encoding) const
		{
			if (!memcmp(p, boms_[encoding].str, boms_[encoding].len))
				return p + boms_[encoding].len;
			else
				return p;
		}

		void cnv_to_utf32(char* text, const encoding encoding)
		{
			if (encoding == encoding::utf16_le)
			{
				const auto bom = skip_bom(text, encoding::utf16_le);
				cnv_utf16_le(bom);
			}

			if (encoding == encoding::utf16_be)
			{
				char* bom = skip_bom(text, encoding::utf16_be);
				cnv_utf16_be(bom);
			}

			if (encoding == encoding::utf8)
			{
				char* bom = skip_bom(text, encoding::utf8);
				cnv_utf8(bom);
			}

			if (encoding == encoding::utf32_le)
			{
				char* bom = skip_bom(text, encoding::utf32_le);
				cnv_utf32_le(bom);
			}

			if (encoding == encoding::utf32_be)
			{
				char* bom = skip_bom(text, encoding::utf32_be);
				cnv_utf32_be(bom);
			}
		}

		static size_t cnv_utf16_be_len(char* text)
		{
			size_t len = 0;
			auto u16_text = reinterpret_cast<char16_t*>(text);
			while (*u16_text)
			{

				char16_t c = 0;
				const auto t = reinterpret_cast<char*>(u16_text);
				c |= char16_t(*(t + 1)) << 8;
				c |= *t;

				//4 bytes
				if (c >= 0xD800 && c <= 0xDFFF)
					u16_text++;

				len++;
				u16_text++;
			}

			return len;
		}

		static size_t cnv_utf16_le_len(char* text)
		{
			size_t len = 0;
			auto u16_text = reinterpret_cast<char16_t*>(text);
			while (*u16_text)
			{
				if (*u16_text >= 0xD800 && *u16_text <= 0xDFFF)
					u16_text++;

				len++;
				u16_text++;
			}

			return len;
		}

		void cnv_utf16_be(char* text)
		{
			const auto s = cnv_utf16_be_len(text);
			utf32_text_ = new char32_t[s + 1];
			utf32_text_[s] = 0;
			auto i = 0;

			auto u16_text = reinterpret_cast<char16_t*>(text);
			char32_t k;

			while (*u16_text)
			{

				char16_t c = 0;
				auto t = reinterpret_cast<char*>(u16_text);
				c |= char16_t(*(t + 1)) << 8;
				c |= *t;

				//4 bytes
				if (c >= 0xD800 && c <= 0xDFFF)
				{
					const char16_t hi = c - 0xD800;

					t = reinterpret_cast<char*>(u16_text + 1);
					c |= char16_t(*(t + 1)) << 8;
					c |= *t;
					const char16_t lo = c - 0xDC00;

					k = char32_t(hi & 0x3FF) << 10;
					k |= lo & 0x3FF;
					k += 0x10000;
					u16_text++;
				}
				else //2 bytes
				{
					k = c;
				}
				utf32_text_[i] = k;
				i++;
				u16_text++;
			}
		}

		void cnv_utf16_le(char* text)
		{
			const auto s = cnv_utf16_le_len(text);
			utf32_text_ = new char32_t[s + 1];
			utf32_text_[s] = 0;
			auto i = 0;

			auto* u16_text = reinterpret_cast<char16_t*>(text);
			char32_t k;

			while (*u16_text)
			{
				//4 bytes
				if (*u16_text >= 0xD800 && *u16_text <= 0xDFFF)
				{
					const char16_t hi = *u16_text - 0xD800;
					const char16_t lo = *(u16_text + 1) - 0xDC00;
					k = char32_t(hi & 0x3FF) << 10;
					k |= lo & 0x3FF;
					k += 0x10000;
					u16_text++;
				}
				else //2 bytes
				{
					k = *u16_text;
				}
				utf32_text_[i] = k;
				i++;
				u16_text++;
			}
		}

		static size_t cnv_utf8_len(char* text)
		{
			size_t len = 0;
			auto c = static_cast<unsigned char>(*text);

			while (c)
			{
				if (c >= 0xc0 && c <= 0xdf)
				{
					text++;
				}
				if (c >= 0xe0 && c <= 0xef)
				{
					text += 2;
				}
				if (c >= 0xf0 && c <= 0xf7)
				{
					text += 3;
				}

				len++;
				c = *++text;
			}

			return len;
		}

		void cnv_utf8(char* text)
		{
			const auto s = cnv_utf8_len(text);
			utf32_text_ = new char32_t[s + 1];
			utf32_text_[s] = 0;
			auto i = 0;

			auto c = static_cast<unsigned char>(*text);

			while (c)
			{
				char32_t k = 0;
				if (c >= 0 && c <= 0x7f)
				{
					k = c;
				}
				if (c >= 0xc0 && c <= 0xdf)
				{
					k = char32_t(c ^ 0xc0) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				if (c >= 0xe0 && c <= 0xef)
				{
					k = char32_t(c ^ 0xe0) << 12;
					c = *++text;
					k |= char32_t(c ^ 0x80) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				if (c >= 0xf0 && c <= 0xf7)
				{
					k = char32_t(c ^ 0xf0) << 18;
					c = *++text;
					k |= char32_t(c ^ 0x80) << 12;
					c = *++text;
					k |= char32_t(c ^ 0x80) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				utf32_text_[i] = k;
				i++;
				c = *++text;
			}
		}

		static size_t cnv_utf32_len(char* text)
		{
			size_t len = 0;
			auto p = reinterpret_cast<char32_t*>(text);

			while (*p)
			{
				len++;
				p++;
			}

			return len;
		}

		void cnv_utf32_le(char* text)
		{
			const auto s = cnv_utf32_len(text);
			utf32_text_ = new char32_t[s + 1];
			utf32_text_[s] = 0;
			auto i = 0;

			auto p = reinterpret_cast<char32_t*>(text);

			while (*p)
			{
				utf32_text_[i] = *p;
				i++;
				p++;
			}
		}

		void cnv_utf32_be(char* text)
		{
			const auto s = cnv_utf32_len(text);
			utf32_text_ = new char32_t[s + 1];
			utf32_text_[s] = 0;
			auto i = 0;

			auto p = reinterpret_cast<char32_t*>(text);

			while (*p)
			{
				const auto t = reinterpret_cast<char*>(p);
				char32_t k = char32_t(*(t + 3)) << 24;
				k |= char32_t(*(t + 2)) << 16;
				k |= char32_t(*(t + 1)) << 8;
				k |= *t;

				utf32_text_[i] = k;
				i++;
				p++;
			}
		}

		static char32_t* trim_space(char32_t* p)
		{
			while (*p)
			{
				if (*p == ' ')
					p++;
				else
					return p;
			}

			return nullptr;
		}

		bool identify(std::shared_ptr<node>& node)
		{
			const auto string_begin = str::parse_text_expected_tag(p_, STRING, true);
			if (!string_begin)
				return false;

			const auto string_end = str::parse_text_end_tag(string_begin + 1, STRING);
			if (!string_end)
				return false;


			const auto object_open = str::parse_text_expected_tag(string_end + 1, OBJECT_OPEN, true);
			const auto val_start = str::parse_text_expected_tag(string_end + 1, STRING, true);

			if (!object_open && !val_start)
				return false;

			if (object_open)
			{
				const auto obj = std::make_shared<object>(this);
				obj->name._start = string_begin + 1;
				obj->name._end = string_end - 1;
				node = obj;
				p_ = object_open + 1;
				return true;
			}

			if (val_start)
			{
				const auto val_end = str::parse_text_end_tag(val_start + 1, STRING);
				if (!val_end)
					return false;

				const auto key_val = std::make_shared<key_value>(this);
				key_val->key._start = string_begin + 1;
				key_val->key._end = string_end - 1;
				key_val->value._start = val_start + 1;
				key_val->value._end = val_end - 1;
				node = key_val;
				p_ = val_end + 1;
				return true;
			}

			return false;
		}

		bool parse_deep()
		{
			root = std::make_shared<node>(this);
			return root->parse();
		}
	};
}