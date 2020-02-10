#pragma once

#pragma region [macro & Attributes & Using section]

// macro
#define DISABLED_COPY(type) public: type(const type&) = delete; type& operator=(const type&) = delete;
#define DISABLED_MOVE(type) public: type(type&&) = delete; type& operator=(type&&) = delete;

// Attributes
#define	_NORETURN		[[noreturn]]
#define	_DEPRECATED		[[deprecated]]
#define	_MAYBE_UNUSED	[[maybe_unused]]
#define	_FALLTHROUGH	[[fallthrough]]
#define	_NODISCARD		[[nodiscard]]

using _Key = short;
using _Pos = short;

enum class DIRECTION : char
{
	LEFT = 0,
	RIGHT = 1
};
#pragma endregion


#pragma region [const]
constexpr unsigned short SERVER_LISTEN_PORT_NUMBER = 9000;
constexpr _Pos MAX_MAP_SIZE = 15;
constexpr _Key MAX_USER = 10;
#pragma endregion


#pragma region [protocol]
#pragma pack(push, 1)

namespace PACKET
{
	namespace SERVER_TO_CLIENT_SINGLE
	{
		struct ADD_OBJECT
		{
			_Pos xPos;
		};

		struct MOVE_OBJECT
		{
			_Pos xPos;
		};
	}

	namespace CLIENT_TO_SERVER_SINGLE
	{
		struct MOVE_OBJECT
		{
			DIRECTION dir;
		};
	}
	
	namespace SERVER_TO_CLIENT
	{
		struct ADD_OBJECT
		{
			_Key key;
			_Pos xPos;
		};

		struct MOVE_OBJECT
		{
			_Key key;
			_Pos xPos;
		};

		struct DELETE_OBJECT
		{
			_Key key;
			_Pos PADDING_0;
		};

		struct ALL_OBJECT_INFO
		{
			_Pos position[10];
		};
	}

	namespace CLIENT_TO_SERVER
	{
		struct MOVE_OBJECT
		{
			DIRECTION dir;
		};
	}
}

#pragma pack(pop)
#pragma endregion
