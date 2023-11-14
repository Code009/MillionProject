#include "pch.h"

using namespace System;

//int main(/*array<System::String ^> ^args*/)
//{
//    return 0;
//}

namespace CLRConsole
{
	class cClass1
	{
	public:
		void Func1(void);

	};
	void StaticFunc1(void){
		
	}
}

#pragma warning(disable:4483)

void __clrcall __identifier(".cctor")(void){
}

__declspec(noinline)
int __clrcall CLREntry(void){
	return 0;
}

//#pragma unmanaged
void CLRConsole::cClass1::Func1(void)
{
}
