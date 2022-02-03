#include "parser.h"

namespace valve_parser
{
	object::object(document* doc) : node(doc) {}

	object* object::to_object()
	{
		return this;
	}

	std::shared_ptr<node> object::get_key_by_name(char* name)
	{
		for (auto& child : children)
		{
			if (child->to_key_value())
			{
				if (util::str_equ(child->to_key_value()->key, name))
					return child;
			}
		}
		return nullptr;
	}

	bool object::parse()
	{
		std::shared_ptr<node> n;

		while (*_doc->p_)
		{
			//check for object close
			const auto string_begin = str::parse_text_expected_tag(_doc->p_, STRING, true);
			if (!string_begin)
			{
				const auto obj_close = str::parse_text_expected_tag(_doc->p_, OBJECT_CLOSE, true);
				if (obj_close)
				{
					_doc->p_ = obj_close + 1;
					return true;
				}
				else
					return false;
			}

			if (!_doc->identify(n))
				return false;

			if (n->to_key_value())
			{
				this->children.push_back(n);
			}

			if (n->to_object())
			{
				this->children.push_back(n);
				object* obj = n->to_object();
				if (!obj->parse())
					return false;
			}
		}
		return false;
	}

	bool node::parse()
	{
		std::shared_ptr<node> n;

		while (*_doc->p_)
		{
			if (!_doc->identify(n))
			{
				if (!str::end_reached(_doc->p_, OBJECT_OPEN) &&
					!str::end_reached(_doc->p_, OBJECT_CLOSE) &&
					!str::end_reached(_doc->p_, STRING))
					return true;
				else
					return false;
			}

			if (n->to_key_value())
			{
				this->children.push_back(n);
			}

			if (n->to_object())
			{
				this->children.push_back(n);
				object* obj = n->to_object();
				if (!obj->parse())
					return false;
			}
		}
		return false;
	}
}