class CfgPatches
{
	class asm
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"A3_Functions_F"};
		fileName = "asm.pbo";
		author = "Fred41";
	};
};
class cfgFunctions
{
	class A3
	{
		class ArmaServerMonitor
		{
			file = "asm";
			class asm
			{
				ext = ".fsm";
				preInit = 1;
			};
		};
	};
};
