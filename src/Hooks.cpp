#include "Hooks.h"
#include "Distribute.h"

namespace Hooks
{
	struct InitItemImpl
	{
		static void thunk(RE::TESObjectBOOK* a_this)
		{
			func(a_this);

			// if InitItem fires after DataLoaded
		    if (Keyword::books) {
		        Distribute::distribute(a_this, Keyword::books.GetKeywords());
			}
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx = 0x13;
	};

	void Install()
	{
		logger::info("{:*^50}", "HOOKS");

		stl::write_vfunc<RE::TESObjectBOOK, InitItemImpl>();

		logger::info("Installed TESObjectBOOK InitItemImpl hook");
	}
}
