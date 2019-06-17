#pragma once
#ifndef INJECTOR_HAS_INJECTORADDONS_HPP
#define INJECTOR_HAS_INJECTORADDONS_HPP

namespace injector
{
	struct reg_pack
	{
		// The ordering is very important, don't change
		// The first field is the last to be pushed and first to be poped

		// PUSHFD / POPFD
		uint32_t ef;

		// PUSHAD/POPAD -- must be the lastest fields (because of esp)
		union
		{
			uint32_t arr[8];
			struct { uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; };
		};

		enum reg_name {
			reg_edi, reg_esi, reg_ebp, reg_esp, reg_ebx, reg_edx, reg_ecx, reg_eax
		};

		enum ef_flag {
			carry_flag = 0, parity_flag = 2, adjust_flag = 4, zero_flag = 6, sign_flag = 7,
			direction_flag = 10, overflow_flag = 11
		};

		uint32_t& operator[](size_t i)
		{
			return this->arr[i];
		}
		const uint32_t& operator[](size_t i) const
		{
			return this->arr[i];
		}

		template<uint32_t bit>   // bit starts from 0, use ef_flag enum
		bool flag()
		{
			return (this->ef & (1 << bit)) != 0;
		}

		bool jnb()
		{
			return flag<carry_flag>() == false;
		}
	};
}

namespace injectorAddons{
	namespace injectorAddons_asm
	{
		// Wrapper functor, so the assembly can use some templating
		template<class T>
		struct wrapper
		{
			static void call(injector::reg_pack* regs)
			{
				T fun; fun(*regs);
			}

			static uintptr_t retOriginalCall(uintptr_t *ptr)
			{
				static uintptr_t originalCall;

				uintptr_t oldcall = originalCall;

				if (ptr != nullptr)
					originalCall = *ptr;

				return oldcall;
			}
		};

		static void *retcall;

		// Constructs a reg_pack and calls the wrapper functor
		template<class W>   // where W is of type wrapper
		inline void __declspec(naked) make_reg_pack_and_call_ret_to_original()
		{
			_asm
			{
				// Construct the reg_pack structure on the stack
				pushad              // Pushes general purposes registers to reg_pack
					add[esp + 12], 4     // Add 4 to reg_pack::esp 'cuz of our return pointer, let it be as before this func is called
					pushfd              // Pushes EFLAGS to reg_pack

					// Call wrapper sending reg_pack as parameter
					push esp
					call W::call
					add esp, 4

					push 0
					call W::retOriginalCall
					mov retcall, eax
					add esp, 4

					// Destructs the reg_pack from the stack
					sub[esp + 12 + 4], 4   // Fix reg_pack::esp before popping it (doesn't make a difference though) (+4 because eflags)
					popfd               // Warning: Do not use any instruction that changes EFLAGS after this (-> sub affects EF!! <-)
					popad

					// Back to normal flow
					push retcall
					ret
			}
		}
	};




	template<uintptr_t at, uintptr_t end, class FuncT>
	void MakeInlineAutoCallOriginal(FuncT func)
	{
		static FuncT static_func = func;    // Stores the func object
		memcpy(&static_func, &func, sizeof(FuncT));                 //

		// Encapsulates the call to static_func
		struct Caps
		{
			void operator()(injector::reg_pack& regs)
			{
				static_func(regs);
			}
		};

		// Does the actual MakeInline
		return MakeInlineAutoCallOriginal<Caps>(injector::lazy_pointer<at>::get(), injector::lazy_pointer<end>::get());
	}

	template<class FuncT>
	void MakeInlineAutoCallOriginal(injector::memory_pointer_tr at)
	{
		typedef injectorAddons_asm::wrapper<FuncT> functor;
		if (false) functor::call(nullptr);   // To instantiate the template, if not done _asm will fail
		uintptr_t originalptr = 0;
		void *test = injector::MakeCALL(at, injectorAddons_asm::make_reg_pack_and_call_ret_to_original<functor>).get();

		originalptr = (uintptr_t)test;

		if (originalptr == 0)
		{
			char errMsg[1024] = {0};
			DWORD readBytes = GetModuleFileName(0, errMsg, sizeof(errMsg) - 1);
			errMsg[1023] = 0;

			MessageBoxA(0, "MakeInlineAutoCallOriginal originalptr is 0", errMsg, 0);
			throw std::runtime_error("originalptr at " __FILE__ " is 0");
		}

		functor::retOriginalCall(&originalptr);
	}

	template<class FuncT>
	void MakeInlineAutoCallOriginal(injector::memory_pointer_tr at, injector::memory_pointer_tr end)
	{
		//injector::MakeRangedNOP(at, end);
		MakeInlineAutoCallOriginal<FuncT>(at);
	}

	template<uintptr_t at, class FuncT>
	void MakeInlineAutoCallOriginal(FuncT func)
	{
		return MakeInlineAutoCallOriginal<at, at + 5, FuncT>(func);
	}
};


#endif
