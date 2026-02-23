
struct identifier
{
	short id;
	short salt;
};
static_assert(sizeof(identifier) == 0x4);