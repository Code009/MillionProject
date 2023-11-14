#include <cnRTL\cnRTL.h>
#include <cnSystem\cnWindows.h>

using namespace cnLibrary;


void TestMain(void)
{
	cnWindows::Initialize();



	cnWindows::Finalize();

}

int main()
{
	TestMain();
	return 0;
}

