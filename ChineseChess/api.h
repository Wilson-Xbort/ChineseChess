#pragma once
#ifndef _API_H_INCLUDED
#define _API_H_INCLUDED

struct API_Structure {
	void(*register_event)(int type, void* addr);
};

#endif // !_API_H_INCLUDED
