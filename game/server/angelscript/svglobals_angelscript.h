//========= Copyright (c) 2023 Johan Ehrendahl, All rights reserved. ============//

#ifndef GAME_SERVER_MSC_ANGELSCRIPT_SVGLOBALS_H
#define GAME_SERVER_MSC_ANGELSCRIPT_SVGLOBALS_H

// This is it's own header file, that reads the AS Manager.
// Since if we add it where MS code is, it's going to explode.
namespace MSC
{
	namespace ASManager
	{
		void Initialize();
		void Shutdown();
		void SetReady( bool );
		void Think();
		void WorldCreated( const char * const );
		void WorldEnded();
	}
}

#endif