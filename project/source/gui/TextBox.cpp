#include "Pch.h"
#include "Base.h"
#include "KeyStates.h"
#include "Scrollbar.h"
#include "TextBox.h"

//-----------------------------------------------------------------------------
TEX TextBox::tBox;
static const int padding = 4;
static const INT2 NOT_SELECTED = INT2(-1, -1);

//=================================================================================================
TextBox::TextBox(bool is_new) : Control(is_new), added(false), multiline(false), numeric(false), label(nullptr), scrollbar(nullptr), readonly(false), caret_index(-1),
	select_start_index(-1), down(false), offset(0), offset_move(0.f), tBackground(nullptr), require_scrollbar(false)
{
}

//=================================================================================================
TextBox::~TextBox()
{
	delete scrollbar;
}

//=================================================================================================
void TextBox::Draw(ControlDrawData* cdd)
{
	TEX background = tBackground ? tBackground : tBox;

	if(!is_new)
	{
		cstring txt = (caret_blink >= 0.f ? Format("%s|", text.c_str()) : text.c_str());

		GUI.DrawItem(background, global_pos, size, WHITE, 4, 32);

		RECT r = { global_pos.x + padding, global_pos.y + padding, global_pos.x + size.x - padding, global_pos.y + size.y - padding };

		if(!scrollbar)
			GUI.DrawText(GUI.default_font, txt, multiline ? DT_TOP : DT_VCENTER, BLACK, r);
		else
		{
			RECT r2 = { r.left, r.top - int(scrollbar->offset), r.right, r.bottom - int(scrollbar->offset) };
			GUI.DrawText(GUI.default_font, txt, DT_TOP, BLACK, r2, &r);
			scrollbar->Draw();
		}

		if(label)
		{
			r.top -= 20;
			GUI.DrawText(GUI.default_font, label, DT_NOCLIP, BLACK, r);
		}
	}
	else
	{
		// not coded yet
		assert(!label);

		BOX2D* clip_rect = nullptr;
		if(cdd)
			clip_rect = cdd->clipping;
		int offsety = (scrollbar ? (int)scrollbar->offset : 0);
		const int line_height = GUI.default_font->height;

		// background
		GUI.DrawItem(background, global_pos, real_size, WHITE, 4, 32, clip_rect);

		RECT rclip;
		RECT textbox_rect = { global_pos.x + padding, global_pos.y + padding, global_pos.x + real_size.x - padding, global_pos.y + real_size.y - padding };
		if(clip_rect)
			IntersectRect(&rclip, &clip_rect->ToRect(), &textbox_rect);
		else
			rclip = textbox_rect;

		// selected
		if(select_start_index != NOT_SELECTED && select_start_index != select_end_index)
		{
			DWORD color = COLOR_RGBA(0, 148, 255, 128);
			int select_start_line = select_start_pos.y / line_height;
			int select_end_line = select_end_pos.y / line_height;
			int lines = select_end_line - select_start_line + 1;
			RECT area, r;
			INT2 pos = global_pos - INT2(offset, offsety) + INT2(padding, padding);

			// ...A----B
			// C-------D
			// E----F...
			if(lines == 1)
			{
				// ...A----B... partial line
				r = {
					pos.x + select_start_pos.x,
					pos.y + select_start_pos.y,
					pos.x + select_end_pos.x,
					pos.y + select_start_pos.y + line_height
				};
				IntersectRect(&area, &r, &rclip);
				GUI.DrawArea(color, INT2(area.left, area.top), INT2(area.right - area.left, area.bottom - area.top));
			}
			else
			{
				// A-B partial top line
				r = {
					pos.x + select_start_pos.x,
					pos.y + select_start_pos.y,
					pos.x + real_size.x,
					pos.y + select_start_pos.y + line_height
				};
				IntersectRect(&area, &r, &rclip);
				GUI.DrawArea(color, INT2(area.left, area.top), INT2(area.right - area.left, area.bottom - area.top));

				// C-D full middle line(s)
				if(lines > 2)
				{
					r = {
						pos.x,
						pos.y + select_start_pos.y + line_height,
						pos.x + real_size.x,
						pos.y + select_end_pos.y
					};
					IntersectRect(&area, &r, &rclip);
					GUI.DrawArea(color, INT2(area.left, area.top), INT2(area.right - area.left, area.bottom - area.top));
				}

				// E-F partial bottom line
				r = {
					pos.x,
					pos.y + select_end_pos.y,
					pos.x + select_end_pos.x,
					pos.y + select_end_pos.y + line_height
				};
				IntersectRect(&area, &r, &rclip);
				GUI.DrawArea(color, INT2(area.left, area.top), INT2(area.right - area.left, area.bottom - area.top));
			}
		}

		// text
		RECT r =
		{
			global_pos.x + padding - offset,
			global_pos.y + padding - offsety,
			global_pos.x + real_size.x - padding,
			global_pos.y + real_size.y - padding
		};
		RECT area;
		IntersectRect(&area, &r, &rclip);
		int draw_flags = (multiline ? DT_LEFT : DT_VCENTER | DT_SINGLELINE);
		GUI.DrawText(GUI.default_font, text, draw_flags, BLACK, r, &area);

		// carret
		if(caret_blink >= 0.f && !readonly)
		{
			INT2 p(global_pos.x + padding + caret_pos.x - offset, global_pos.y + padding + caret_pos.y - offsety);
			if(p.x >= rclip.left && p.x <= rclip.right)
				GUI.DrawArea(BLACK, p, INT2(1, line_height), clip_rect);
		}

		if(require_scrollbar)
			scrollbar->Draw();
	}
}

//=================================================================================================
void TextBox::Update(float dt)
{
	bool clicked = false;

	if(mouse_focus)
	{
		if(!readonly && PointInRect(GUI.cursor_pos, global_pos, real_size))
		{
			GUI.cursor_mode = CURSOR_TEXT;
			if(is_new && (Key.PressedRelease(VK_LBUTTON) || Key.PressedRelease(VK_RBUTTON)))
			{
				// set caret position, update selection
				bool prev_focus = focus;
				INT2 new_index, new_pos;
				GetCaretPos(GUI.cursor_pos, new_index, new_pos);
				caret_blink = 0.f;
				TakeFocus(true);
				if(Key.Down(VK_SHIFT) && prev_focus)
					CalculateSelection(new_index, new_pos);
				else
					select_start_index = NOT_SELECTED;
				caret_index = new_index;
				caret_pos = new_pos;
				down = true;
				clicked = true;
			}
		}
		if(scrollbar && !is_new)
			scrollbar->Update(dt);
	}

	if(scrollbar && is_new)
	{
		if(mouse_focus)
			scrollbar->ApplyMouseWheel();
		UpdateControl(scrollbar, dt);
	}

	if(focus)
	{
		// update caret blinking
		caret_blink += dt * 2;
		if(caret_blink >= 1.f)
			caret_blink = -1.f;

		if(is_new)
		{
			// update selecting with mouse
			if(down && !clicked)
			{
				if(Key.Up(VK_LBUTTON))
					down = false;
				else
				{
					const int MOVE_SPEED = 300;

					if(!multiline)
					{
						int local_x = GUI.cursor_pos.x - global_pos.x - padding;
						if(local_x <= 0.1f * size.x && offset != 0)
						{
							offset_move -= dt * MOVE_SPEED;
							int offset_move_i = -(int)ceil(offset_move);
							offset_move += offset_move_i;
							offset -= offset_move_i;
							if(offset < 0)
								offset = 0;
						}
						else if(local_x >= 0.9f * size.x)
						{
							offset_move += dt * MOVE_SPEED;
							int offset_move_i = (int)floor(offset_move);
							offset_move -= offset_move_i;
							offset += offset_move_i;
							const int real_size = size.x - padding * 2;
							const int total_width = GUI.default_font->CalculateSize(text).x;
							int max_offset = total_width - real_size;
							if(offset > max_offset)
								offset = max_offset;
							if(offset < 0)
								offset = 0;
						}
					}
					else
					{
						int local_y = GUI.cursor_pos.y - global_pos.y - padding;
						if(local_y <= 0.1f * size.y)
						{

						}
						else if(local_y >= 0.9f * size.y)
						{

						}
					}

					INT2 new_index, new_pos;
					GetCaretPos(GUI.cursor_pos, new_index, new_pos);
					if(new_index != caret_index)
					{
						CalculateSelection(new_index, new_pos);
						caret_index = new_index;
						caret_pos = new_pos;
						caret_blink = 0.f;
					}
				}
			}

			/*
			if(Key.DownRepeat(VK_DELETE))
			{
				if(select_start_index != NOT_SELECTED)
				{
					DeleteSelection();
					CalculateOffset(true);
				}
				else if(!multiline)
				{
					if(caret_index.x != text.size())
					{
						text.erase(caret_index.x, 1);
						caret_blink = 0.f;
						CalculateOffset(true);
					}
				}
				else
				{
					if(caret_index.y + 1 != font_lines.size() || caret_index.x != font_lines[caret_index.y].count)
					{
						uint raw_index = ToRawIndex(caret_index);
						text.erase(raw_index, 1);
						caret_blink = 0.f;
						UpdateText();
					}
				}
			}*/

			// move caret
			int move = 0;
			if(Key.DownRepeat(VK_UP))
				move -= 10;
			if(Key.DownRepeat(VK_DOWN))
				move += 10;
			if(move == 0)
			{
				if(Key.DownRepeat(VK_LEFT))
					move -= 1;
				if(Key.DownRepeat(VK_RIGHT))
					move += 1;
			}

			if(move < 0)
			{
				if(!(caret_index.x > 0 || caret_index.y > 0))
				{
					move = 0;
					if(select_start_index != NOT_SELECTED)
					{
						select_start_index = NOT_SELECTED;
						CalculateOffset(false);
					}
				}
			}
			else if(move > 0)
			{
				if(!(caret_index.y + 1 != font_lines.size() || caret_index.x != font_lines[caret_index.y].count))
				{
					move = 0;
					if(select_start_index != NOT_SELECTED)
					{
						select_start_index = NOT_SELECTED;
						CalculateOffset(false);
					}
				}
			}

			if(move != 0)
			{
				INT2 new_index, new_pos;
				if(Key.Down(VK_SHIFT) || select_start_index == NOT_SELECTED || move == 10 || move == -10)
				{
					select_start_index = NOT_SELECTED;
					switch(move)
					{
					case -10:
						// TODO
						assert(0);
						break;
					case -1:
						if(caret_index.x > 0)
						{
							new_index = INT2(caret_index.x - 1, caret_index.y);
							uint raw_index = ToRawIndex(new_index);
							new_pos = caret_pos;
							new_pos.x -= GUI.default_font->GetCharWidth(text[raw_index]);
						}
						else
						{
							assert(caret_index.y > 0);
							new_index = INT2(font_lines[caret_index.y - 1].count, caret_index.y - 1);
							new_pos = INT2(font_lines[new_index.y].width, new_index.y * GUI.default_font->height);
						}
						break;
					case +1:
						if((uint)caret_index.x < font_lines[caret_index.y].count)
						{
							new_index = INT2(caret_index.x + 1, caret_index.y);
							uint raw_index = ToRawIndex(new_index);
							new_pos = caret_pos;
							new_pos.x += GUI.default_font->GetCharWidth(text[raw_index - 1]);
						}
						else
						{
							assert((uint)caret_index.y < font_lines.size());
							new_index = INT2(0, caret_index.y + 1);
							new_pos = INT2(0, new_index.y * GUI.default_font->height);
						}
						break;
					case +10:
						// TODO
						assert(0);
						break;
					}

					if(Key.Down(VK_SHIFT))
						CalculateSelection(new_index, new_pos);
				}
				else if(select_start_index != NOT_SELECTED)
				{
					if(move == -1)
					{
						new_index = select_start_index;
						new_pos = select_start_pos;
					}
					else
					{
						new_index = select_end_index;
						new_pos = select_end_pos;
					}
					select_start_index = NOT_SELECTED;
				}
				
				caret_index = new_index;
				caret_pos = new_pos;
				caret_blink = 0.f;
				CalculateOffset(false);
			}

			// select all
			/*if(Key.Shortcut(KEY_CONTROL, 'A'))
			{
				caret_index = INT2(0, 0);
				caret_pos = INT2(0, 0);
				caret_blink = 0.f;
				select_start_index = INT2(0, 0);
				select_start_pos = INT2(0, 0);
				select_end_index = INT2(font_lines.back().count + 1, font_lines.size());
				select_end_pos = INT2(font_lines.back().width, font_lines.size() * GUI.default_font->height);
				select_fixed_index = INT2(0, 0);
			}

			// copy
			if(select_start_index != NOT_SELECTED && Key.Shortcut(KEY_CONTROL, 'C'))
			{
				uint start = ToRawIndex(select_start_index);
				uint end = ToRawIndex(select_end_index);
				GUI.SetClipboard(text.substr(start, end - start).c_str());
			}

			// paste
			if(!readonly && Key.Shortcut(KEY_CONTROL, 'V'))
			{
				cstring clipboard = GUI.GetClipboard();
				if(clipboard)
				{
					if(select_start_index != NOT_SELECTED)
						DeleteSelection();
					int len = strlen(clipboard);
					if(limit <= 0 || text.length() + len <= (uint)limit)
						text.insert(caret_index, clipboard);
					else
					{
						int max_chars = limit - text.length();
						text.insert(caret_index, clipboard, max_chars);
					}
					caret_index += strlen(clipboard);
					caret_pos = IndexToPos(caret_index);
					CalculateOffset(true);
				}
			}

			// cut
			if(!readonly && select_start_index != -1 && Key.Shortcut(KEY_CONTROL, 'X'))
			{
				GUI.SetClipboard(text.substr(select_start_index, select_end_index - select_start_index).c_str());
				DeleteSelection();
				CalculateOffset(true);
			}*/
		}
	}
	else
	{
		caret_blink = -1.f;
		down = false;
	}
}

//=================================================================================================
void TextBox::Event(GuiEvent e)
{
	switch(e)
	{
	case GuiEvent_Moved:
		if(scrollbar && is_new)
			scrollbar->global_pos = scrollbar->pos + global_pos;
		break;
	case GuiEvent_Resize:
		if(scrollbar && is_new)
		{
			scrollbar->pos = INT2(size.x - 16, 0);
			scrollbar->size = INT2(16, size.y);
			scrollbar->part = size.y - 8;
			UpdateScrollbarVisibility();
		}
		break;
	case GuiEvent_GainFocus:
		if(!added)
		{
			if(!is_new)
				caret_blink = 0.f;
			if(!readonly)
				GUI.AddOnCharHandler(this);
			added = true;
		}
		break;
	case GuiEvent_LostFocus:
		if(added)
		{
			select_start_index = NOT_SELECTED;
			caret_blink = -1.f;
			if(!readonly)
				GUI.RemoveOnCharHandler(this);
			added = false;
			down = false;
			offset_move = 0.f;
		}
		break;
	case GuiEvent_Initialize:
		if(multiline && is_new)
		{
			scrollbar = new Scrollbar;
			scrollbar->pos = INT2(size.x - 16, 0);
			scrollbar->size = INT2(16, size.y);
			scrollbar->total = 0;
			scrollbar->part = size.y - 8;
			scrollbar->offset = 0.f;
			UpdateScrollbarVisibility();
			UpdateFontLines();
		}
		break;
	}
}

//=================================================================================================
void TextBox::OnChar(char c)
{
	if(c == VK_BACK)
	{
		// backspace
		if(!is_new)
		{
			if(!text.empty())
				text.pop_back();
		}
		else
		{
			if(select_start_index != NOT_SELECTED)
			{
				DeleteSelection();
				CalculateOffset(true);
			}
			//else if(caret_index > 0)
			{
				assert(0);
				/*--caret_index;
				caret_pos -= GUI.default_font->GetCharWidth(text[caret_index]);
				caret_blink = 0.f;
				text.erase(caret_index, 1);
				CalculateOffset(true);*/
			}
		}
	}
	else if(c == VK_RETURN && !multiline)
	{
		// enter - skip
	}
	else
	{
		if(numeric)
		{
			assert(!is_new);
			if(c == '-')
			{
				if(text.empty())
					text.push_back('-');
				else if(text[0] == '-')
					text.erase(text.begin());
				else
					text.insert(text.begin(), '-');
				ValidateNumber();
			}
			else if(c == '+')
			{
				if(!text.empty() && text[0] == '-')
				{
					text.erase(text.begin());
					ValidateNumber();
				}
			}
			else if(isdigit(byte(c)))
			{
				int len = text.length();
				if(!text.empty() && text[0] == '-')
					--len;
				text.push_back(c);
				ValidateNumber();
			}
		}
		else
		{
			if(select_start_index != NOT_SELECTED)
				DeleteSelection();
			if(limit <= 0 || limit > (int)text.size())
			{
				if(!is_new)
					text.push_back(c);
				else
				{
					assert(0);
					/*text.insert(caret_index, 1, c);
					caret_pos += GUI.default_font->GetCharWidth(c);
					caret_blink = 0.f;
					++caret_index;
					CalculateOffset(true);*/
				}
			}
		}
	}
}

//=================================================================================================
void TextBox::ValidateNumber()
{
	int n = atoi(text.c_str());
	if(n < num_min)
		text = Format("%d", num_min);
	else if(n > num_max)
		text = Format("%d", num_max);
}

//=================================================================================================
void TextBox::AddScrollbar()
{
	assert(!is_new); // use SetMultiline
	if(scrollbar)
		return;
	scrollbar = new Scrollbar;
	scrollbar->pos = INT2(size.x + 2, 0);
	scrollbar->size = INT2(16, size.y);
	scrollbar->total = 8;
	scrollbar->part = size.y - 8;
	scrollbar->offset = 0.f;
}

//=================================================================================================
void TextBox::Move(const INT2& _global_pos)
{
	global_pos = _global_pos + pos;
	if(scrollbar)
		scrollbar->global_pos = global_pos + scrollbar->pos;
}

//=================================================================================================
void TextBox::Add(cstring str)
{
	assert(!is_new);
	assert(scrollbar);
	INT2 str_size = GUI.default_font->CalculateSize(str, size.x - 8);
	bool skip_to_end = (int(scrollbar->offset) >= (scrollbar->total - scrollbar->part));
	scrollbar->total += str_size.y;
	if(text.empty())
		text = str;
	else
	{
		text += '\n';
		text += str;
	}
	if(skip_to_end)
	{
		scrollbar->offset = float(scrollbar->total - scrollbar->part);
		if(scrollbar->offset < 0.f)
			scrollbar->offset = 0.f;
	}
}

//=================================================================================================
void TextBox::Reset()
{
	text.clear();
	if(scrollbar)
	{
		scrollbar->total = 8;
		scrollbar->part = size.y - 8;
		scrollbar->offset = 0.f;
	}
	caret_index = INT2(0, 0);
	caret_pos = INT2(0, 0);
	select_start_index = NOT_SELECTED;
}

//=================================================================================================
void TextBox::UpdateScrollbar()
{
	INT2 text_size = GUI.default_font->CalculateSize(text, size.x - 8);
	scrollbar->total = text_size.y;
}

//=================================================================================================
void TextBox::UpdateSize(const INT2& new_pos, const INT2& new_size)
{
	global_pos = pos = new_pos;
	size = new_size;

	scrollbar->global_pos = scrollbar->pos = INT2(global_pos.x + size.x - 16, global_pos.y);
	scrollbar->size = INT2(16, size.y);
	scrollbar->offset = 0.f;
	scrollbar->part = size.y - 8;

	UpdateScrollbar();
}

//=================================================================================================
void TextBox::GetCaretPos(const INT2& in_pos, INT2& out_index, INT2& out_pos)
{
	const INT2 real_size_without_pad = real_size - INT2(padding, padding) * 2;
	uint index;
	INT2 index2;
	IBOX2D rect;
	float uv;

	if(!scrollbar)
	{
		int local_x = in_pos.x - global_pos.x - padding + offset;
		if(local_x < 0)
		{
			out_index = INT2(0, 0);
			out_pos = INT2(0, 0);
			return;
		}

		if(local_x > real_size_without_pad.x + offset)
			local_x = real_size_without_pad.x + offset;
		else if(local_x < offset)
			local_x = offset;

		GUI.default_font->HitTest(text, real_size_without_pad.x, DT_SINGLELINE | DT_VCENTER, INT2(local_x, 0), index, index2, rect, uv);
	}
	else
	{
		int offsety = (int)scrollbar->offset;
		int local_x = in_pos.x - global_pos.x - padding + offset;
		int local_y = in_pos.y - global_pos.y - padding + offsety;
		if(local_x < 0)
			local_x = 0;
		if(local_y < 0)
			local_y = 0;

		if(local_x > real_size_without_pad.x + offset)
			local_x = real_size_without_pad.x + offset;
		else if(local_x < offset)
			local_x = offset;
		if(local_y > real_size_without_pad.y + offsety)
			local_y = real_size_without_pad.y + offsety;
		else if(local_y < offsety)
			local_y = offsety;

		GUI.default_font->HitTest(text, real_size_without_pad.x, DT_LEFT, INT2(local_x, local_y), index, index2, rect, uv);
	}

	if(uv >= 0.5f)
	{
		out_index = INT2(index2.x + 1, index2.y);
		out_pos = INT2(rect.p2.x, rect.p1.y);
	}
	else
	{
		out_index = index2;
		out_pos = rect.p1;
	}
}

//=================================================================================================
void TextBox::CalculateSelection(const INT2& new_index, const INT2& new_pos)
{
	if(select_start_index == NOT_SELECTED)
	{
		CalculateSelection(new_index, new_pos, caret_index, caret_pos);
		select_fixed_index = caret_index;
	}
	else if(select_fixed_index == select_start_index)
		CalculateSelection(new_index, new_pos, select_start_index, select_start_pos);
	else
		CalculateSelection(new_index, new_pos, select_end_index, select_end_pos);
}

//=================================================================================================
void TextBox::CalculateSelection(INT2 index1, INT2 pos1, INT2 index2, INT2 pos2)
{
	if(index1.y < index2.y || (index1.x < index2.x && index1.y == index2.y))
	{
		select_start_index = index1;
		select_start_pos = pos1;
		select_end_index = index2;
		select_end_pos = pos2;
	}
	else
	{
		select_start_index = index2;
		select_start_pos = pos2;
		select_end_index = index1;
		select_end_pos = pos1;
	}
}

//=================================================================================================
void TextBox::DeleteSelection()
{
	/*text.erase(select_start_index, select_end_index - select_start_index);
	caret_index = select_start_index;
	caret_pos = select_start_pos;
	caret_blink = 0.f;
	select_start_index = -1;*/
	assert(0);
}

//=================================================================================================
INT2 TextBox::IndexToPos(int index)
{
	const INT2 real_size_without_pad = real_size - INT2(padding, padding) * 2;
	int flags;
	if(scrollbar)
		flags = DT_LEFT;
	else
		flags = DT_SINGLELINE | DT_VCENTER;
	return GUI.default_font->IndexToPos(index, text, real_size_without_pad.x, flags);
}

//=================================================================================================
void TextBox::SetText(cstring new_text)
{
	if(new_text)
		text = new_text;
	else
		text.clear();
	select_start_index = NOT_SELECTED;
	offset = 0;
	UpdateScrollbarVisibility();
	UpdateFontLines();
}

//=================================================================================================
void TextBox::CalculateOffset(bool center)
{
	if(!scrollbar)
	{
		const int real_pos = caret_pos.x - offset;
		const int real_size = size.x - padding * 2;
		const int total_width = GUI.default_font->CalculateSize(text).x;
		if(real_pos < 0 || real_pos >= real_size)
		{
			if(center)
				offset = caret_pos.x - real_size / 2;
			else if(real_pos < 0)
				offset = caret_pos.x;
			else
				offset = caret_pos.x - real_size;
		}

		int max_offset = total_width - real_size;
		if(offset > max_offset)
			offset = max_offset;
		if(offset < 0)
			offset = 0;
	}
	else
	{
		const int real_size = size.y - padding * 2;
		int offsety = (int)scrollbar->offset;
		int local_y = caret_pos.y - offsety;
		if(local_y < 0)
			scrollbar->offset = (float)caret_pos.y;
		else if(local_y + GUI.default_font->height > real_size)
			scrollbar->offset = (float)(caret_pos.y + GUI.default_font->height - real_size);
	}
}

//=================================================================================================
void TextBox::SelectAll()
{
	SetFocus();
	if(text.empty())
	{
		caret_index = INT2(0, 0);
		caret_pos = INT2(0, 0);
		select_start_index = NOT_SELECTED;
	}
	else
	{
		caret_index = INT2(font_lines.back().count, font_lines.size());
		caret_pos = INT2(font_lines.back().width, font_lines.size() * GUI.default_font->height);
		select_start_index = INT2(0, 0);
		select_end_index = caret_index;
		select_start_pos = INT2(0, 0);
		select_end_pos = caret_pos;
		select_fixed_index = INT2(0, 0);
	}
}

//=================================================================================================
void TextBox::UpdateScrollbarVisibility()
{
	if(!initialized)
		return;
	if(!scrollbar)
	{
		real_size = size;
		return;
	}
	text_size = GUI.default_font->CalculateSize(text, size.x - 15 - padding * 2);
	scrollbar->UpdateTotal(text_size.y);
	require_scrollbar = scrollbar->IsRequired();
	if(require_scrollbar)
		real_size = INT2(size.x - 15, size.y);
	else
		real_size = size;
}

uint TextBox::ToRawIndex(const INT2& index)
{
	assert(index.x >= 0 && index.y >= 0 && index.y < (int)font_lines.size());
	auto& line = font_lines[index.y];
	assert(index.x <= (int)line.count);
	return line.begin + index.x;
}

void TextBox::UpdateFontLines()
{
	if(!initialized)
		return;
	int flags;
	if(multiline)
		flags = DT_LEFT;
	else
		flags = DT_SINGLELINE | DT_VCENTER;
	GUI.default_font->PrecalculateFontLines(font_lines, text, size.x - 15 - padding * 2, flags);
}