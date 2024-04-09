#include <dumper.h>

int main(int argc, char* argv[])
{
	std::unique_ptr<Dumper> dumper(new Dumper(argc, argv));
	return dumper->result;
}
