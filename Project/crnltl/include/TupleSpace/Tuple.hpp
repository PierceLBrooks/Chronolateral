#ifndef _TUPLE_HPP_
#define _TUPLE_HPP_

#include <cstdarg>
#include <string>
#include <vector>
#include <map>

/// A C++ wrapper for a Python tuple
class Tuple
{
    /***** NESTED CLASSES *****/
    public:
        /// A parent struct for items
        struct BasicItem
        {
            /// Basic constructor
            BasicItem() {};

            /// Basic destructor
            virtual ~BasicItem() {};
        };

        /// A specialized struct for items
        template<typename T> struct Item : BasicItem
        {
            /// The tuple item itself
            T mItem;

            /// Basic constructor
            Item<T>(T item) : mItem(item) {};

            /// Basic destructor
            ~Item<T>() {};
        };

    /***** CONSTRUCTORS / DESTRUCTORS *****/
    public:
        /// Empty constructor
        Tuple() {}

        /// Constructor which builds the item list from a variable number of arguments and code for their respective types
        Tuple(std::string code, ...);

        /// Constructor which builds the item list and then appends items from a given tuple
        Tuple(Tuple * tuple, std::string code, ...);

        /// Destructor which destroys the item list
        virtual ~Tuple();

    /***** METHODS *****/
    public:

        /// Returns a copy of an item at a given index interpretted as a paricular type
        bool getItemAsBool(unsigned int index);
        int getItemAsInt(unsigned int index);
        float getItemAsFloat(unsigned int index);
        double getItemAsDouble(unsigned int index);
        char getItemAsChar(unsigned int index);
        long getItemAsLong(unsigned int index);
        unsigned int getItemAsUnsignedInt(unsigned int index);
        std::string getItemAsString(unsigned int index);
        void* getItemAsVoid(unsigned int index);

        /// Returns the size of the item list
        unsigned int getSize() const;

        /// Returns the code of the item list
        const std::string& getCode() const;

    /***** ATTRIBUTES *****/
    protected:
        /// The code for the arguments that were initially passed to the constructor
        std::string mCode;

        /// The item list
        std::vector<BasicItem*> mItems;

        /// Master list of compatible types
        static std::map<char, std::vector<char>> mCompatibleTypes;
};

#endif
