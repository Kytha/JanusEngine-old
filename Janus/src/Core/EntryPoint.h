#include "Debug/Instrumentor.h";

extern Janus::Application* Janus::CreateApplication();
// ENTRY POINT
int main(int argc, char** argv)
{
	Janus::Log::Init();
	
	JN_PROFILE_BEGIN_SESSION("Startup", "JanusProfile-Startup.json");
	auto app = Janus::CreateApplication();
	JN_PROFILE_END_SESSION();

	JN_PROFILE_BEGIN_SESSION("Runtime", "JanusProfile-Runtime.json");
	app->Run();
	JN_PROFILE_END_SESSION();

	JN_PROFILE_BEGIN_SESSION("Shutdown", "JanusProfile-Shutdown.json");
	delete app;
	JN_PROFILE_END_SESSION();
}