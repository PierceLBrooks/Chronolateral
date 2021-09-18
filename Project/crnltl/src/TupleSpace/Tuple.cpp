#include <TupleSpace/Tuple.hpp>

Tuple::Tuple(std::string code, ...) :
	mCode(code)
{
	va_list list;
	va_start(list, code);
	for (int i = 0; i != mCode.size(); ++i)
	{
		BasicItem * item = nullptr;
		switch (mCode[i])
		{
		case 'b':
			item = new Item<bool>(va_arg(list, int)!=0);
			break;
		case 'i':
			item = new Item<int>(va_arg(list, int));
			break;
		case 'f':
			item = new Item<float>(static_cast<float>(va_arg(list, double)));
			break;
		case 'd':
			item = new Item<double>(va_arg(list, double));
			break;
		case 'c':
			item = new Item<char>(static_cast<char>(va_arg(list, int)));
			break;
		case 'l':
			item = new Item<long>(va_arg(list, long));
			break;
		case 'u':
			item = new Item<unsigned int>(static_cast<unsigned int>(va_arg(list, int)));
			break;
		case 's':
			item = new Item<std::string>(std::string(va_arg(list, char*)));
			break;
		case 'v':
			item = new Item<void*>(va_arg(list, void*));
			break;
		default:
			if (i == mItems.size() - 1)
			{
				mCode = mCode.substr(0, i);
				break;
			}
			mCode = mCode.substr(0, i) + mCode.substr(i + 1);
			--i;
			continue;
		}
		if (item != nullptr)
		{
			mItems.push_back(item);
		}
	}
	va_end(list);
}

Tuple::Tuple(Tuple * tuple, std::string code, ...) :
	mCode(code)
{
	va_list list;
	va_start(list, code);
	for (int i = 0; i != mCode.size(); ++i)
	{
		BasicItem * item = nullptr;
		switch (mCode[i])
		{
		case 'b':
			item = new Item<bool>(va_arg(list, int)!=0);
			break;
		case 'i':
			item = new Item<int>(va_arg(list, int));
			break;
		case 'f':
			item = new Item<float>(static_cast<float>(va_arg(list, double)));
			break;
		case 'd':
			item = new Item<double>(va_arg(list, double));
			break;
		case 'c':
			item = new Item<char>(static_cast<char>(va_arg(list, int)));
			break;
		case 'l':
			item = new Item<long>(va_arg(list, long));
			break;
		case 'u':
			item = new Item<unsigned int>(static_cast<unsigned int>(va_arg(list, int)));
			break;
		case 's':
			item = new Item<std::string>(std::string(va_arg(list, char*)));
			break;
		default:
			if (i == mItems.size() - 1)
			{
				mCode = mCode.substr(0, i);
				break;
			}
			mCode = mCode.substr(0, i) + mCode.substr(i + 1);
			--i;
			continue;
		}
		if (item != nullptr)
		{
			mItems.push_back(item);
		}
	}
	if (tuple)
	{
		code = tuple->getCode();
		for (int i = 0; i != tuple->getSize(); ++i)
		{
			BasicItem * item = nullptr;
			switch (code[i])
			{
			case 'b':
				item = new Item<bool>(tuple->getItemAsBool(i));
				break;
			case 'i':
				item = new Item<int>(tuple->getItemAsInt(i));
				break;
			case 'f':
				item = new Item<float>(tuple->getItemAsFloat(i));
				break;
			case 'd':
				item = new Item<double>(tuple->getItemAsDouble(i));
				break;
			case 'c':
				item = new Item<char>(tuple->getItemAsChar(i));
				break;
			case 'l':
				item = new Item<long>(tuple->getItemAsLong(i));
				break;
			case 'u':
				item = new Item<unsigned int>(tuple->getItemAsUnsignedInt(i));
				break;
			case 's':
				item = new Item<std::string>(tuple->getItemAsString(i));
				break;
			default:
				if (i == mItems.size() - 1)
				{
					code = code.substr(0, i);
					break;
				}
				code = code.substr(0, i) + code.substr(i + 1);
				--i;
				continue;
			}
			if (item != nullptr)
			{
				mItems.push_back(item);
				mCode.push_back(code[i]);
			}
		}
	}
	va_end(list);
}

Tuple::~Tuple()
{
	for (int i = 0; i != mItems.size(); ++i)
	{
		delete mItems[i];
	}
	mItems.clear();
}

bool Tuple::getItemAsBool(unsigned int index)
{
	bool error = false;
	char code = 'b';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<bool>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<bool>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<bool>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<bool>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<bool>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<bool>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<bool>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<bool>(atoi(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

int Tuple::getItemAsInt(unsigned int index)
{
	int error = 0;
	char code = 'i';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<int>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<int>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<int>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<int>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<int>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<int>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<int>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<int>(atoi(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

float Tuple::getItemAsFloat(unsigned int index)
{
	float error = 0.0f;
	char code = 'f';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<float>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<float>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<float>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<float>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<float>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<float>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<float>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<float>(atof(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

double Tuple::getItemAsDouble(unsigned int index)
{
	double error = 0.0;
	char code = 'd';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<double>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<double>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<double>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<double>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<double>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<double>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<double>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<double>(atof(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

char Tuple::getItemAsChar(unsigned int index)
{
	char error = 0;
	char code = 'c';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<char>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<char>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<char>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<char>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<char>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<char>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<char>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<char>(atoi(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

long Tuple::getItemAsLong(unsigned int index)
{
	long error = 0;
	char code = 'l';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<long>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<long>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<long>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<long>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<long>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<long>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<long>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<long>(atoi(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

unsigned int Tuple::getItemAsUnsignedInt(unsigned int index)
{
	unsigned int error = 0;
	char code = 'u';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return static_cast<unsigned int>(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return static_cast<unsigned int>(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return static_cast<unsigned int>(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return static_cast<unsigned int>(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return static_cast<unsigned int>(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return static_cast<unsigned int>(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return static_cast<unsigned int>(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<unsigned int>(atoi(static_cast<Item<std::string>*>(mItems[index])->mItem.c_str()));
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

std::string Tuple::getItemAsString(unsigned int index)
{
	std::string error = "";
	char code = 's';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] == 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'b':
		return std::to_string(static_cast<Item<bool>*>(mItems[index])->mItem);
	case 'i':
		return std::to_string(static_cast<Item<int>*>(mItems[index])->mItem);
	case 'f':
		return std::to_string(static_cast<Item<float>*>(mItems[index])->mItem);
	case 'd':
		return std::to_string(static_cast<Item<double>*>(mItems[index])->mItem);
	case 'c':
		return std::to_string(static_cast<Item<char>*>(mItems[index])->mItem);
	case 'l':
		return std::to_string(static_cast<Item<long>*>(mItems[index])->mItem);
	case 'u':
		return std::to_string(static_cast<Item<unsigned int>*>(mItems[index])->mItem);
	case 's':
		return static_cast<Item<std::string>*>(mItems[index])->mItem;
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

void* Tuple::getItemAsVoid(unsigned int index)
{
	void* error = nullptr;
	char code = 'v';
	if (index >= mItems.size())
	{
		return error;
	}
	if (mCode[index] != 'v')
	{
		return error;
	}
	switch (mCode[index])
	{
	case 'v':
		return static_cast<Item<void*>*>(mItems[index])->mItem;
	default:
		if (index == mItems.size() - 1)
		{
			mCode = mCode.substr(0, index);
			break;
		}
		mCode = mCode.substr(0, index) + mCode.substr(index + 1);
		break;
	}
	return error;
}

unsigned int Tuple::getSize() const
{
	return mItems.size();
}

const std::string& Tuple::getCode() const
{
	return mCode;
}
