// START include/json/value.h
#ifndef CPPTL_JSON_H_INCLUDED
# define CPPTL_JSON_H_INCLUDED

// START include/json/forwards.h
#ifndef JSON_FORWARDS_H_INCLUDED
# define JSON_FORWARDS_H_INCLUDED

// START include/json/config.h
#ifndef JSON_CONFIG_H_INCLUDED
# define JSON_CONFIG_H_INCLUDED

/// If defined, indicates that json library is embedded in CppTL library.
//# define JSON_IN_CPPTL 1

/// If defined, indicates that json may leverage CppTL library
//#  define JSON_USE_CPPTL 1
/// If defined, indicates that cpptl vector based map should be used instead of std::map
/// as Value container.
//#  define JSON_USE_CPPTL_SMALLMAP 1
/// If defined, indicates that Json specific container should be used
/// (hash table & simple deque container with customizable allocator).
/// THIS FEATURE IS STILL EXPERIMENTAL!
//#  define JSON_VALUE_USE_INTERNAL_MAP 1
/// Force usage of standard new/malloc based allocator instead of memory pool based allocator.
/// The memory pools allocator used optimization (initializing Value and ValueInternalLink
/// as if it was a POD) that may cause some validation tool to report errors.
/// Only has effects if JSON_VALUE_USE_INTERNAL_MAP is defined.
//#  define JSON_USE_SIMPLE_INTERNAL_ALLOCATOR 1


# ifdef JSON_IN_CPPTL
#  include <cpptl/config.h>
#  ifndef JSON_USE_CPPTL
#   define JSON_USE_CPPTL 1
#  endif
# endif

# ifdef JSON_IN_CPPTL
#  define JSON_API CPPTL_API
# elif defined(JSON_DLL_BUILD)
#  define JSON_API __declspec(dllexport)
# elif defined(JSON_DLL)
#  define JSON_API __declspec(dllimport)
# else
#  define JSON_API
# endif

#endif // JSON_CONFIG_H_INCLUDED
// END   include/json/config.h

namespace Json {

   class FastWriter;
   class Reader;
   class StyledWriter;

   // value.h
   class StaticString;
   class Path;
   class PathArgument;
   class Value;
   class ValueIteratorBase;
   class ValueIterator;
   class ValueConstIterator;
#ifdef JSON_VALUE_USE_INTERNAL_MAP
   class ValueAllocator;
   class ValueMapAllocator;
   class ValueInternalLink;
   class ValueInternalArray;
   class ValueInternalMap;
#endif // #ifdef JSON_VALUE_USE_INTERNAL_MAP

} // namespace Json


#endif // JSON_FORWARDS_H_INCLUDED
// END   include/json/forwards.h
# include <string>
# include <vector>

# ifndef JSON_USE_CPPTL_SMALLMAP
#  include <map>
# else
#  include <cpptl/smallmap.h>
# endif
# ifdef JSON_USE_CPPTL
#  include <cpptl/forwards.h>
# endif

/** \brief JSON (JavaScript Object Notation).
 */
namespace Json {

   /** \brief Type of the value held by a Value object.
    */
   enum ValueType
   {
      nullValue = 0, ///< 'null' value
      intValue,      ///< signed integer value
      uintValue,     ///< unsigned integer value
      realValue,     ///< double value
      stringValue,   ///< UTF-8 string value
      booleanValue,  ///< bool value
      arrayValue,    ///< array value (ordered list)
      objectValue    ///< object value (collection of name/value pairs).
   };

   enum CommentPlacement
   {
      commentBefore = 0,        ///< a comment placed on the line before a value
      commentAfterOnSameLine,   ///< a comment just after a value on the same line
      commentAfter,             ///< a comment on the line after a value (only make sense for root value)
      numberOfCommentPlacement
   };

//# ifdef JSON_USE_CPPTL
//   typedef CppTL::AnyEnumerator<const char *> EnumMemberNames;
//   typedef CppTL::AnyEnumerator<const Value &> EnumValues;
//# endif

   /** \brief Lightweight wrapper to tag static string.
    *
    * Value constructor and objectValue member assignement takes advantage of the
    * StaticString and avoid the cost of string duplication when storing the
    * string or the member name.
    *
    * Example of usage:
    * \code
    * Json::Value aValue( StaticString("some text") );
    * Json::Value object;
    * static const StaticString code("code");
    * object[code] = 1234;
    * \endcode
    */
   class JSON_API StaticString
   {
   public:
      explicit StaticString( const char *czstring )
         : str_( czstring )
      {
      }

      operator const char *() const
      {
         return str_;
      }

      const char *c_str() const
      {
         return str_;
      }

   private:
      const char *str_;
   };

   /** \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
    *
    * This class is a discriminated union wrapper that can represents a:
    * - signed integer [range: Value::minInt - Value::maxInt]
    * - unsigned integer (range: 0 - Value::maxUInt)
    * - double
    * - UTF-8 string
    * - boolean
    * - 'null'
    * - an ordered list of Value
    * - collection of name/value pairs (javascript object)
    *
    * The type of the held value is represented by a #ValueType and 
    * can be obtained using type().
    *
    * values of an #objectValue or #arrayValue can be accessed using operator[]() methods. 
    * Non const methods will automatically create the a #nullValue element 
    * if it does not exist. 
    * The sequence of an #arrayValue will be automatically resize and initialized 
    * with #nullValue. resize() can be used to enlarge or truncate an #arrayValue.
    *
    * The get() methods can be used to obtanis default value in the case the required element
    * does not exist.
    *
    * It is possible to iterate over the list of a #objectValue values using 
    * the getMemberNames() method.
    */
   class JSON_API Value 
   {
      friend class ValueIteratorBase;
# ifdef JSON_VALUE_USE_INTERNAL_MAP
      friend class ValueInternalLink;
      friend class ValueInternalMap;
# endif
   public:
      typedef std::vector<std::string> Members;
      typedef int Int;
      typedef unsigned int UInt;
      typedef ValueIterator iterator;
      typedef ValueConstIterator const_iterator;
      typedef UInt ArrayIndex;

      static const Value null;
      static const Int minInt;
      static const Int maxInt;
      static const UInt maxUInt;

   private:
#ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION
# ifndef JSON_VALUE_USE_INTERNAL_MAP
      class CZString 
      {
      public:
         enum DuplicationPolicy 
         {
            noDuplication = 0,
            duplicate,
            duplicateOnCopy
         };
         CZString( int index );
         CZString( const char *cstr, DuplicationPolicy allocate );
         CZString( const CZString &other );
         ~CZString();
         CZString &operator =( const CZString &other );
         bool operator<( const CZString &other ) const;
         bool operator==( const CZString &other ) const;
         int index() const;
         const char *c_str() const;
         bool isStaticString() const;
      private:
         void swap( CZString &other );
         const char *cstr_;
         int index_;
      };

   public:
#  ifndef JSON_USE_CPPTL_SMALLMAP
      typedef std::map<CZString, Value> ObjectValues;
#  else
      typedef CppTL::SmallMap<CZString, Value> ObjectValues;
#  endif // ifndef JSON_USE_CPPTL_SMALLMAP
# endif // ifndef JSON_VALUE_USE_INTERNAL_MAP
#endif // ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION

   public:
      /** \brief Create a default Value of the given type.

        This is a very useful constructor.
        To create an empty array, pass arrayValue.
        To create an empty object, pass objectValue.
        Another Value can then be set to this one by assignment.
	This is useful since clear() and resize() will not alter types.

        Examples:
	\code
	Json::Value null_value; // null
	Json::Value arr_value(Json::arrayValue); // []
	Json::Value obj_value(Json::objectValue); // {}
	\endcode
      */
      Value( ValueType type = nullValue );
      Value( Int value );
      Value( UInt value );
      Value( double value );
      Value( const char *value );
      /** \brief Constructs a value from a static string.

       * Like other value string constructor but do not duplicate the string for
       * internal storage. The given string must remain alive after the call to this
       * constructor.
       * Example of usage:
       * \code
       * Json::Value aValue( StaticString("some text") );
       * \endcode
       */
      Value( const StaticString &value );
      Value( const std::string &value );
# ifdef JSON_USE_CPPTL
      Value( const CppTL::ConstString &value );
# endif
      Value( bool value );
      Value( const Value &other );
      ~Value();

      Value &operator=( const Value &other );
      /// Swap values.
      /// \note Currently, comments are intentionally not swapped, for
      /// both logic and efficiency.
      void swap( Value &other );

      ValueType type() const;

      bool operator <( const Value &other ) const;
      bool operator <=( const Value &other ) const;
      bool operator >=( const Value &other ) const;
      bool operator >( const Value &other ) const;

      bool operator ==( const Value &other ) const;
      bool operator !=( const Value &other ) const;

      int compare( const Value &other );

      const char *asCString() const;
      std::string asString() const;
# ifdef JSON_USE_CPPTL
      CppTL::ConstString asConstString() const;
# endif
      Int asInt() const;
      UInt asUInt() const;
      double asDouble() const;
      bool asBool() const;

      bool isNull() const;
      bool isBool() const;
      bool isInt() const;
      bool isUInt() const;
      bool isIntegral() const;
      bool isDouble() const;
      bool isNumeric() const;
      bool isString() const;
      bool isArray() const;
      bool isObject() const;

      bool isConvertibleTo( ValueType other ) const;

      /// Number of values in array or object
      UInt size() const;

      /// \brief Return true if empty array, empty object, or null;
      /// otherwise, false.
      bool empty() const;

      /// Return isNull()
      bool operator!() const;

      /// Remove all object members and array elements.
      /// \pre type() is arrayValue, objectValue, or nullValue
      /// \post type() is unchanged
      void clear();

      /// Resize the array to size elements. 
      /// New elements are initialized to null.
      /// May only be called on nullValue or arrayValue.
      /// \pre type() is arrayValue or nullValue
      /// \post type() is arrayValue
      void resize( UInt size );

      /// Access an array element (zero based index ).
      /// If the array contains less than index element, then null value are inserted
      /// in the array so that its size is index+1.
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      Value &operator[]( UInt index );
      /// Access an array element (zero based index )
      /// (You may need to say 'value[0u]' to get your compiler to distinguish
      ///  this from the operator[] which takes a string.)
      const Value &operator[]( UInt index ) const;
      /// If the array contains at least index+1 elements, returns the element value, 
      /// otherwise returns defaultValue.
      Value get( UInt index, 
                 const Value &defaultValue ) const;
      /// Return true if index < size().
      bool isValidIndex( UInt index ) const;
      /// \brief Append value to array at the end.
      ///
      /// Equivalent to jsonvalue[jsonvalue.size()] = value;
      Value &append( const Value &value );

      /// Access an object value by name, create a null member if it does not exist.
      Value &operator[]( const char *key );
      /// Access an object value by name, returns null if there is no member with that name.
      const Value &operator[]( const char *key ) const;
      /// Access an object value by name, create a null member if it does not exist.
      Value &operator[]( const std::string &key );
      /// Access an object value by name, returns null if there is no member with that name.
      const Value &operator[]( const std::string &key ) const;
      /** \brief Access an object value by name, create a null member if it does not exist.

       * If the object as no entry for that name, then the member name used to store
       * the new entry is not duplicated.
       * Example of use:
       * \code
       * Json::Value object;
       * static const StaticString code("code");
       * object[code] = 1234;
       * \endcode
       */
      Value &operator[]( const StaticString &key );
# ifdef JSON_USE_CPPTL
      /// Access an object value by name, create a null member if it does not exist.
      Value &operator[]( const CppTL::ConstString &key );
      /// Access an object value by name, returns null if there is no member with that name.
      const Value &operator[]( const CppTL::ConstString &key ) const;
# endif
      /// Return the member named key if it exist, defaultValue otherwise.
      Value get( const char *key, 
                 const Value &defaultValue ) const;
      /// Return the member named key if it exist, defaultValue otherwise.
      Value get( const std::string &key,
                 const Value &defaultValue ) const;
# ifdef JSON_USE_CPPTL
      /// Return the member named key if it exist, defaultValue otherwise.
      Value get( const CppTL::ConstString &key,
                 const Value &defaultValue ) const;
# endif
      /// \brief Remove and return the named member.  
      ///
      /// Do nothing if it did not exist.
      /// \return the removed Value, or null.
      /// \pre type() is objectValue or nullValue
      /// \post type() is unchanged
      Value removeMember( const char* key );
      /// Same as removeMember(const char*)
      Value removeMember( const std::string &key );

      /// Return true if the object has a member named key.
      bool isMember( const char *key ) const;
      /// Return true if the object has a member named key.
      bool isMember( const std::string &key ) const;
# ifdef JSON_USE_CPPTL
      /// Return true if the object has a member named key.
      bool isMember( const CppTL::ConstString &key ) const;
# endif

      /// \brief Return a list of the member names.
      ///
      /// If null, return an empty list.
      /// \pre type() is objectValue or nullValue
      /// \post if type() was nullValue, it remains nullValue
      Members getMemberNames() const;

//# ifdef JSON_USE_CPPTL
//      EnumMemberNames enumMemberNames() const;
//      EnumValues enumValues() const;
//# endif

      /// Comments must be //... or /* ... */
      void setComment( const char *comment,
                       CommentPlacement placement );
      /// Comments must be //... or /* ... */
      void setComment( const std::string &comment,
                       CommentPlacement placement );
      bool hasComment( CommentPlacement placement ) const;
      /// Include delimiters and embedded newlines.
      std::string getComment( CommentPlacement placement ) const;

      std::string toStyledString() const;

      const_iterator begin() const;
      const_iterator end() const;

      iterator begin();
      iterator end();

   private:
      Value &resolveReference( const char *key, 
                               bool isStatic );

# ifdef JSON_VALUE_USE_INTERNAL_MAP
      inline bool isItemAvailable() const
      {
         return itemIsUsed_ == 0;
      }

      inline void setItemUsed( bool isUsed = true )
      {
         itemIsUsed_ = isUsed ? 1 : 0;
      }

      inline bool isMemberNameStatic() const
      {
         return memberNameIsStatic_ == 0;
      }

      inline void setMemberNameIsStatic( bool isStatic )
      {
         memberNameIsStatic_ = isStatic ? 1 : 0;
      }
# endif // # ifdef JSON_VALUE_USE_INTERNAL_MAP

   private:
      struct CommentInfo
      {
         CommentInfo();
         ~CommentInfo();

         void setComment( const char *text );

         char *comment_;
      };

      //struct MemberNamesTransform
      //{
      //   typedef const char *result_type;
      //   const char *operator()( const CZString &name ) const
      //   {
      //      return name.c_str();
      //   }
      //};

      union ValueHolder
      {
         Int int_;
         UInt uint_;
         double real_;
         bool bool_;
         char *string_;
# ifdef JSON_VALUE_USE_INTERNAL_MAP
         ValueInternalArray *array_;
         ValueInternalMap *map_;
#else
         ObjectValues *map_;
# endif
      } value_;
      ValueType type_ : 8;
      int allocated_ : 1;     // Notes: if declared as bool, bitfield is useless.
# ifdef JSON_VALUE_USE_INTERNAL_MAP
      unsigned int itemIsUsed_ : 1;      // used by the ValueInternalMap container.
      int memberNameIsStatic_ : 1;       // used by the ValueInternalMap container.
# endif
      CommentInfo *comments_;
   };


   /** \brief Experimental and untested: represents an element of the "path" to access a node.
    */
   class PathArgument
   {
   public:
      friend class Path;

      PathArgument();
      PathArgument( Value::UInt index );
      PathArgument( const char *key );
      PathArgument( const std::string &key );

   private:
      enum Kind
      {
         kindNone = 0,
         kindIndex,
         kindKey
      };
      std::string key_;
      Value::UInt index_;
      Kind kind_;
   };

   /** \brief Experimental and untested: represents a "path" to access a node.
    *
    * Syntax:
    * - "." => root node
    * - ".[n]" => elements at index 'n' of root node (an array value)
    * - ".name" => member named 'name' of root node (an object value)
    * - ".name1.name2.name3"
    * - ".[0][1][2].name1[3]"
    * - ".%" => member name is provided as parameter
    * - ".[%]" => index is provied as parameter
    */
   class Path
   {
   public:
      Path( const std::string &path,
            const PathArgument &a1 = PathArgument(),
            const PathArgument &a2 = PathArgument(),
            const PathArgument &a3 = PathArgument(),
            const PathArgument &a4 = PathArgument(),
            const PathArgument &a5 = PathArgument() );

      const Value &resolve( const Value &root ) const;
      Value resolve( const Value &root, 
                     const Value &defaultValue ) const;
      /// Creates the "path" to access the specified node and returns a reference on the node.
      Value &make( Value &root ) const;

   private:
      typedef std::vector<const PathArgument *> InArgs;
      typedef std::vector<PathArgument> Args;

      void makePath( const std::string &path,
                     const InArgs &in );
      void addPathInArg( const std::string &path, 
                         const InArgs &in, 
                         InArgs::const_iterator &itInArg, 
                         PathArgument::Kind kind );
      void invalidPath( const std::string &path, 
                        int location );

      Args args_;
   };

   /** \brief Allocator to customize member name and string value memory management done by Value.
    *
    * - makeMemberName() and releaseMemberName() are called to respectively duplicate and
    *   free an Json::objectValue member name.
    * - duplicateStringValue() and releaseStringValue() are called similarly to
    *   duplicate and free a Json::stringValue value.
    */
   class ValueAllocator
   {
   public:
      enum { unknown = (unsigned)-1 };

      virtual ~ValueAllocator();

      virtual char *makeMemberName( const char *memberName ) = 0;
      virtual void releaseMemberName( char *memberName ) = 0;
      virtual char *duplicateStringValue( const char *value, 
                                          unsigned int length = unknown ) = 0;
      virtual void releaseStringValue( char *value ) = 0;
   };

#ifdef JSON_VALUE_USE_INTERNAL_MAP
   /** \brief Allocator to customize Value internal map.
    * Below is an example of a simple implementation (default implementation actually
    * use memory pool for speed).
    * \code
      class DefaultValueMapAllocator : public ValueMapAllocator
      {
      public: // overridden from ValueMapAllocator
         virtual ValueInternalMap *newMap()
         {
            return new ValueInternalMap();
         }

         virtual ValueInternalMap *newMapCopy( const ValueInternalMap &other )
         {
            return new ValueInternalMap( other );
         }

         virtual void destructMap( ValueInternalMap *map )
         {
            delete map;
         }

         virtual ValueInternalLink *allocateMapBuckets( unsigned int size )
         {
            return new ValueInternalLink[size];
         }

         virtual void releaseMapBuckets( ValueInternalLink *links )
         {
            delete [] links;
         }

         virtual ValueInternalLink *allocateMapLink()
         {
            return new ValueInternalLink();
         }

         virtual void releaseMapLink( ValueInternalLink *link )
         {
            delete link;
         }
      };
    * \endcode
    */ 
   class JSON_API ValueMapAllocator
   {
   public:
      virtual ~ValueMapAllocator();
      virtual ValueInternalMap *newMap() = 0;
      virtual ValueInternalMap *newMapCopy( const ValueInternalMap &other ) = 0;
      virtual void destructMap( ValueInternalMap *map ) = 0;
      virtual ValueInternalLink *allocateMapBuckets( unsigned int size ) = 0;
      virtual void releaseMapBuckets( ValueInternalLink *links ) = 0;
      virtual ValueInternalLink *allocateMapLink() = 0;
      virtual void releaseMapLink( ValueInternalLink *link ) = 0;
   };

   /** \brief ValueInternalMap hash-map bucket chain link (for internal use only).
    * \internal previous_ & next_ allows for bidirectional traversal.
    */
   class JSON_API ValueInternalLink
   {
   public:
      enum { itemPerLink = 6 };  // sizeof(ValueInternalLink) = 128 on 32 bits architecture.
      enum InternalFlags { 
         flagAvailable = 0,
         flagUsed = 1
      };

      ValueInternalLink();

      ~ValueInternalLink();

      Value items_[itemPerLink];
      char *keys_[itemPerLink];
      ValueInternalLink *previous_;
      ValueInternalLink *next_;
   };


   /** \brief A linked page based hash-table implementation used internally by Value.
    * \internal ValueInternalMap is a tradional bucket based hash-table, with a linked
    * list in each bucket to handle collision. There is an addional twist in that
    * each node of the collision linked list is a page containing a fixed amount of
    * value. This provides a better compromise between memory usage and speed.
    * 
    * Each bucket is made up of a chained list of ValueInternalLink. The last
    * link of a given bucket can be found in the 'previous_' field of the following bucket.
    * The last link of the last bucket is stored in tailLink_ as it has no following bucket.
    * Only the last link of a bucket may contains 'available' item. The last link always
    * contains at least one element unless is it the bucket one very first link.
    */
   class JSON_API ValueInternalMap
   {
      friend class ValueIteratorBase;
      friend class Value;
   public:
      typedef unsigned int HashKey;
      typedef unsigned int BucketIndex;

# ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION
      struct IteratorState
      {
         ValueInternalMap *map_;
         ValueInternalLink *link_;
         BucketIndex itemIndex_;
         BucketIndex bucketIndex_;
      };
# endif // ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION

      ValueInternalMap();
      ValueInternalMap( const ValueInternalMap &other );
      ValueInternalMap &operator =( const ValueInternalMap &other );
      ~ValueInternalMap();

      void swap( ValueInternalMap &other );

      BucketIndex size() const;

      void clear();

      bool reserveDelta( BucketIndex growth );

      bool reserve( BucketIndex newItemCount );

      const Value *find( const char *key ) const;

      Value *find( const char *key );

      Value &resolveReference( const char *key, 
                               bool isStatic );

      void remove( const char *key );

      void doActualRemove( ValueInternalLink *link, 
                           BucketIndex index,
                           BucketIndex bucketIndex );

      ValueInternalLink *&getLastLinkInBucket( BucketIndex bucketIndex );

      Value &setNewItem( const char *key, 
                         bool isStatic, 
                         ValueInternalLink *link, 
                         BucketIndex index );

      Value &unsafeAdd( const char *key, 
                        bool isStatic, 
                        HashKey hashedKey );

      HashKey hash( const char *key ) const;

      int compare( const ValueInternalMap &other ) const;

   private:
      void makeBeginIterator( IteratorState &it ) const;
      void makeEndIterator( IteratorState &it ) const;
      static bool equals( const IteratorState &x, const IteratorState &other );
      static void increment( IteratorState &iterator );
      static void incrementBucket( IteratorState &iterator );
      static void decrement( IteratorState &iterator );
      static const char *key( const IteratorState &iterator );
      static const char *key( const IteratorState &iterator, bool &isStatic );
      static Value &value( const IteratorState &iterator );
      static int distance( const IteratorState &x, const IteratorState &y );

   private:
      ValueInternalLink *buckets_;
      ValueInternalLink *tailLink_;
      BucketIndex bucketsSize_;
      BucketIndex itemCount_;
   };

   /** \brief A simplified deque implementation used internally by Value.
   * \internal
   * It is based on a list of fixed "page", each page contains a fixed number of items.
   * Instead of using a linked-list, a array of pointer is used for fast item look-up.
   * Look-up for an element is as follow:
   * - compute page index: pageIndex = itemIndex / itemsPerPage
   * - look-up item in page: pages_[pageIndex][itemIndex % itemsPerPage]
   *
   * Insertion is amortized constant time (only the array containing the index of pointers
   * need to be reallocated when items are appended).
   */
   class JSON_API ValueInternalArray
   {
      friend class Value;
      friend class ValueIteratorBase;
   public:
      enum { itemsPerPage = 8 };    // should be a power of 2 for fast divide and modulo.
      typedef Value::ArrayIndex ArrayIndex;
      typedef unsigned int PageIndex;

# ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION
      struct IteratorState // Must be a POD
      {
         ValueInternalArray *array_;
         Value **currentPageIndex_;
         unsigned int currentItemIndex_;
      };
# endif // ifndef JSONCPP_DOC_EXCLUDE_IMPLEMENTATION

      ValueInternalArray();
      ValueInternalArray( const ValueInternalArray &other );
      ValueInternalArray &operator =( const ValueInternalArray &other );
      ~ValueInternalArray();
      void swap( ValueInternalArray &other );

      void clear();
      void resize( ArrayIndex newSize );

      Value &resolveReference( ArrayIndex index );

      Value *find( ArrayIndex index ) const;

      ArrayIndex size() const;

      int compare( const ValueInternalArray &other ) const;

   private:
      static bool equals( const IteratorState &x, const IteratorState &other );
      static void increment( IteratorState &iterator );
      static void decrement( IteratorState &iterator );
      static Value &dereference( const IteratorState &iterator );
      static Value &unsafeDereference( const IteratorState &iterator );
      static int distance( const IteratorState &x, const IteratorState &y );
      static ArrayIndex indexOf( const IteratorState &iterator );
      void makeBeginIterator( IteratorState &it ) const;
      void makeEndIterator( IteratorState &it ) const;
      void makeIterator( IteratorState &it, ArrayIndex index ) const;

      void makeIndexValid( ArrayIndex index );

      Value **pages_;
      ArrayIndex size_;
      PageIndex pageCount_;
   };

   /** \brief Allocator to customize Value internal array.
    * Below is an example of a simple implementation (actual implementation use
    * memory pool).
      \code
class DefaultValueArrayAllocator : public ValueArrayAllocator
{
public: // overridden from ValueArrayAllocator
   virtual ~DefaultValueArrayAllocator()
   {
   }

   virtual ValueInternalArray *newArray()
   {
      return new ValueInternalArray();
   }

   virtual ValueInternalArray *newArrayCopy( const ValueInternalArray &other )
   {
      return new ValueInternalArray( other );
   }

   virtual void destruct( ValueInternalArray *array )
   {
      delete array;
   }

   virtual void reallocateArrayPageIndex( Value **&indexes, 
                                          ValueInternalArray::PageIndex &indexCount,
                                          ValueInternalArray::PageIndex minNewIndexCount )
   {
      ValueInternalArray::PageIndex newIndexCount = (indexCount*3)/2 + 1;
      if ( minNewIndexCount > newIndexCount )
         newIndexCount = minNewIndexCount;
      void *newIndexes = realloc( indexes, sizeof(Value*) * newIndexCount );
      if ( !newIndexes )
         throw std::bad_alloc();
      indexCount = newIndexCount;
      indexes = static_cast<Value **>( newIndexes );
   }
   virtual void releaseArrayPageIndex( Value **indexes, 
                                       ValueInternalArray::PageIndex indexCount )
   {
      if ( indexes )
         free( indexes );
   }

   virtual Value *allocateArrayPage()
   {
      return static_cast<Value *>( malloc( sizeof(Value) * ValueInternalArray::itemsPerPage ) );
   }

   virtual void releaseArrayPage( Value *value )
   {
      if ( value )
         free( value );
   }
};
      \endcode
    */ 
   class JSON_API ValueArrayAllocator
   {
   public:
      virtual ~ValueArrayAllocator();
      virtual ValueInternalArray *newArray() = 0;
      virtual ValueInternalArray *newArrayCopy( const ValueInternalArray &other ) = 0;
      virtual void destructArray( ValueInternalArray *array ) = 0;
      /** \brief Reallocate array page index.
       * Reallocates an array of pointer on each page.
       * \param indexes [input] pointer on the current index. May be \c NULL.
       *                [output] pointer on the new index of at least 
       *                         \a minNewIndexCount pages. 
       * \param indexCount [input] current number of pages in the index.
       *                   [output] number of page the reallocated index can handle.
       *                            \b MUST be >= \a minNewIndexCount.
       * \param minNewIndexCount Minimum number of page the new index must be able to
       *                         handle.
       */
      virtual void reallocateArrayPageIndex( Value **&indexes, 
                                             ValueInternalArray::PageIndex &indexCount,
                                             ValueInternalArray::PageIndex minNewIndexCount ) = 0;
      virtual void releaseArrayPageIndex( Value **indexes, 
                                          ValueInternalArray::PageIndex indexCount ) = 0;
      virtual Value *allocateArrayPage() = 0;
      virtual void releaseArrayPage( Value *value ) = 0;
   };
#endif // #ifdef JSON_VALUE_USE_INTERNAL_MAP


   /** \brief Experimental and untested: base class for Value iterators.
    *
    */
   class ValueIteratorBase
   {
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef ValueIteratorBase SelfType;

      ValueIteratorBase();
#ifndef JSON_VALUE_USE_INTERNAL_MAP
      explicit ValueIteratorBase( const Value::ObjectValues::iterator &current );
#else
      ValueIteratorBase( const ValueInternalArray::IteratorState &state );
      ValueIteratorBase( const ValueInternalMap::IteratorState &state );
#endif

      bool operator ==( const SelfType &other ) const
      {
         return isEqual( other );
      }

      bool operator !=( const SelfType &other ) const
      {
         return !isEqual( other );
      }

      difference_type operator -( const SelfType &other ) const
      {
         return computeDistance( other );
      }

      /// Return either the index or the member name of the referenced value as a Value.
      Value key() const;

      /// Return the index of the referenced Value. -1 if it is not an arrayValue.
      Value::UInt index() const;

      /// Return the member name of the referenced Value. "" if it is not an objectValue.
      const char *memberName() const;

   protected:
      Value &deref() const;

      void increment();

      void decrement();

      difference_type computeDistance( const SelfType &other ) const;

      bool isEqual( const SelfType &other ) const;

      void copy( const SelfType &other );

   private:
#ifndef JSON_VALUE_USE_INTERNAL_MAP
      Value::ObjectValues::iterator current_;
#else
      union
      {
         ValueInternalArray::IteratorState array_;
         ValueInternalMap::IteratorState map_;
      } iterator_;
      bool isArray_;
#endif
   };

   /** \brief Experimental and untested: const iterator for object and array value.
    *
    */
   class ValueConstIterator : public ValueIteratorBase
   {
      friend class Value;
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef const Value &reference;
      typedef const Value *pointer;
      typedef ValueConstIterator SelfType;

      ValueConstIterator();
   private:
      /*! \internal Use by Value to create an iterator.
       */
#ifndef JSON_VALUE_USE_INTERNAL_MAP
      explicit ValueConstIterator( const Value::ObjectValues::iterator &current );
#else
      ValueConstIterator( const ValueInternalArray::IteratorState &state );
      ValueConstIterator( const ValueInternalMap::IteratorState &state );
#endif
   public:
      SelfType &operator =( const ValueIteratorBase &other );

      SelfType operator++( int )
      {
         SelfType temp( *this );
         ++*this;
         return temp;
      }

      SelfType operator--( int )
      {
         SelfType temp( *this );
         --*this;
         return temp;
      }

      SelfType &operator--()
      {
         decrement();
         return *this;
      }

      SelfType &operator++()
      {
         increment();
         return *this;
      }

      reference operator *() const
      {
         return deref();
      }
   };


   /** \brief Experimental and untested: iterator for object and array value.
    */
   class ValueIterator : public ValueIteratorBase
   {
      friend class Value;
   public:
      typedef unsigned int size_t;
      typedef int difference_type;
      typedef Value &reference;
      typedef Value *pointer;
      typedef ValueIterator SelfType;

      ValueIterator();
      ValueIterator( const ValueConstIterator &other );
      ValueIterator( const ValueIterator &other );
   private:
      /*! \internal Use by Value to create an iterator.
       */
#ifndef JSON_VALUE_USE_INTERNAL_MAP
      explicit ValueIterator( const Value::ObjectValues::iterator &current );
#else
      ValueIterator( const ValueInternalArray::IteratorState &state );
      ValueIterator( const ValueInternalMap::IteratorState &state );
#endif
   public:

      SelfType &operator =( const SelfType &other );

      SelfType operator++( int )
      {
         SelfType temp( *this );
         ++*this;
         return temp;
      }

      SelfType operator--( int )
      {
         SelfType temp( *this );
         --*this;
         return temp;
      }

      SelfType &operator--()
      {
         decrement();
         return *this;
      }

      SelfType &operator++()
      {
         increment();
         return *this;
      }

      reference operator *() const
      {
         return deref();
      }
   };


} // namespace Json


#endif // CPPTL_JSON_H_INCLUDED
// END   include/json/value.h
// START include/json/json.h
#ifndef JSON_JSON_H_INCLUDED
# define JSON_JSON_H_INCLUDED

// START include/json/autolink.h
#ifndef JSON_AUTOLINK_H_INCLUDED
# define JSON_AUTOLINK_H_INCLUDED


# ifdef JSON_IN_CPPTL
#  include <cpptl/cpptl_autolink.h>
# endif

# if !defined(JSON_NO_AUTOLINK)  &&  !defined(JSON_DLL_BUILD)  &&  !defined(JSON_IN_CPPTL)
#  define CPPTL_AUTOLINK_NAME "json"
#  undef CPPTL_AUTOLINK_DLL
#  ifdef JSON_DLL
#   define CPPTL_AUTOLINK_DLL
#  endif
# endif

#endif // JSON_AUTOLINK_H_INCLUDED
// END   include/json/autolink.h
// START include/json/reader.h
#ifndef CPPTL_JSON_READER_H_INCLUDED
# define CPPTL_JSON_READER_H_INCLUDED

# include <deque>
# include <stack>
# include <string>
# include <iostream>

namespace Json {

   class Value;

   /** \brief Unserialize a <a HREF="http://www.json.org">JSON</a> document into a Value.
    *
    *
    */
   class JSON_API Reader
   {
   public:
      typedef char Char;
      typedef const Char *Location;

      Reader();

      /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
       * \param document UTF-8 encoded string containing the document to read.
       * \param root [out] Contains the root value of the document if it was
       *             successfully parsed.
       * \param collectComments \c true to collect comment and allow writing them back during
       *                        serialization, \c false to discard comments.
       * \return \c true if the document was successfully parsed, \c false if an error occurred.
       */
      bool parse( const std::string &document, 
                  Value &root,
                  bool collectComments = true );

      /** \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
       * \param document UTF-8 encoded string containing the document to read.
       * \param root [out] Contains the root value of the document if it was
       *             successfully parsed.
       * \param collectComments \c true to collect comment and allow writing them back during
       *                        serialization, \c false to discard comments.
       * \return \c true if the document was successfully parsed, \c false if an error occurred.
       */
      bool parse( const char *beginDoc, const char *endDoc, 
                  Value &root,
                  bool collectComments = true );

      /// \brief Parse from input stream.
      /// \see Json::operator>>(std::istream&, Json::Value&).
      bool parse( std::istream&,
                  Value &root,
                  bool collectComments = true );

      /** \brief Returns a user friendly string that list errors in the parsed document.
       * \return Formatted error message with the list of errors with their location in 
       *         the parsed document. An empty string is returned if no error occurred
       *         during parsing.
       */
      std::string getFormatedErrorMessages() const;

   private:
      enum TokenType
      {
         tokenEndOfStream = 0,
         tokenObjectBegin,
         tokenObjectEnd,
         tokenArrayBegin,
         tokenArrayEnd,
         tokenString,
         tokenNumber,
         tokenTrue,
         tokenFalse,
         tokenNull,
         tokenArraySeparator,
         tokenMemberSeparator,
         tokenComment,
         tokenError
      };

      class Token
      {
      public:
         TokenType type_;
         Location start_;
         Location end_;
      };

      class ErrorInfo
      {
      public:
         Token token_;
         std::string message_;
         Location extra_;
      };

      typedef std::deque<ErrorInfo> Errors;

      bool expectToken( TokenType type, Token &token, const char *message );
      bool readToken( Token &token );
      void skipSpaces();
      bool match( Location pattern, 
                  int patternLength );
      bool readComment();
      bool readCStyleComment();
      bool readCppStyleComment();
      bool readString();
      void readNumber();
      bool readValue();
      bool readObject( Token &token );
      bool readArray( Token &token );
      bool decodeNumber( Token &token );
      bool decodeString( Token &token );
      bool decodeString( Token &token, std::string &decoded );
      bool decodeDouble( Token &token );
      bool decodeUnicodeEscapeSequence( Token &token, 
                                        Location &current, 
                                        Location end, 
                                        unsigned int &unicode );
      bool addError( const std::string &message, 
                     Token &token,
                     Location extra = 0 );
      bool recoverFromError( TokenType skipUntilToken );
      bool addErrorAndRecover( const std::string &message, 
                               Token &token,
                               TokenType skipUntilToken );
      void skipUntilSpace();
      Value &currentValue();
      Char getNextChar();
      void getLocationLineAndColumn( Location location,
                                     int &line,
                                     int &column ) const;
      std::string getLocationLineAndColumn( Location location ) const;
      void addComment( Location begin, 
                       Location end, 
                       CommentPlacement placement );
      void skipCommentTokens( Token &token );
   
      typedef std::stack<Value *> Nodes;
      Nodes nodes_;
      Errors errors_;
      std::string document_;
      Location begin_;
      Location end_;
      Location current_;
      Location lastValueEnd_;
      Value *lastValue_;
      std::string commentsBefore_;
      bool collectComments_;
   };

   /** \brief Read from 'sin' into 'root'.

    Always keep comments from the input JSON.

    This can be used to read a file into a particular sub-object.
    For example:
    \code
    Json::Value root;
    cin >> root["dir"]["file"];
    cout << root;
    \endcode
    Result:
    \verbatim
    {
	"dir": {
	    "file": {
		// The input stream JSON would be nested here.
	    }
	}
    }
    \endverbatim
    \throw std::exception on parse error.
    \see Json::operator<<()
   */
   std::istream& operator>>( std::istream&, Value& );

} // namespace Json

#endif // CPPTL_JSON_READER_H_INCLUDED
// END   include/json/reader.h
// START include/json/writer.h
#ifndef JSON_WRITER_H_INCLUDED
# define JSON_WRITER_H_INCLUDED

# include <vector>
# include <string>
# include <iostream>

namespace Json {

   class Value;

   /** \brief Abstract class for writers.
    */
   class JSON_API Writer
   {
   public:
      virtual ~Writer();

      virtual std::string write( const Value &root ) = 0;
   };

   /** \brief Outputs a Value in <a HREF="http://www.json.org">JSON</a> format without formatting (not human friendly).
    *
    * The JSON document is written in a single line. It is not intended for 'human' consumption,
    * but may be usefull to support feature such as RPC where bandwith is limited.
    * \sa Reader, Value
    */
   class JSON_API FastWriter : public Writer
   {
   public:
      FastWriter();
      virtual ~FastWriter(){}

      void enableYAMLCompatibility();

   public: // overridden from Writer
      virtual std::string write( const Value &root );

   private:
      void writeValue( const Value &value );

      std::string document_;
      bool yamlCompatiblityEnabled_;
   };

   /** \brief Writes a Value in <a HREF="http://www.json.org">JSON</a> format in a human friendly way.
    *
    * The rules for line break and indent are as follow:
    * - Object value:
    *     - if empty then print {} without indent and line break
    *     - if not empty the print '{', line break & indent, print one value per line
    *       and then unindent and line break and print '}'.
    * - Array value:
    *     - if empty then print [] without indent and line break
    *     - if the array contains no object value, empty array or some other value types,
    *       and all the values fit on one lines, then print the array on a single line.
    *     - otherwise, it the values do not fit on one line, or the array contains
    *       object or non empty array, then print one value per line.
    *
    * If the Value have comments then they are outputed according to their #CommentPlacement.
    *
    * \sa Reader, Value, Value::setComment()
    */
   class JSON_API StyledWriter: public Writer
   {
   public:
      StyledWriter();
      virtual ~StyledWriter(){}

   public: // overridden from Writer
      /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
       * \param root Value to serialize.
       * \return String containing the JSON document that represents the root value.
       */
      virtual std::string write( const Value &root );

   private:
      void writeValue( const Value &value );
      void writeArrayValue( const Value &value );
      bool isMultineArray( const Value &value );
      void pushValue( const std::string &value );
      void writeIndent();
      void writeWithIndent( const std::string &value );
      void indent();
      void unindent();
      void writeCommentBeforeValue( const Value &root );
      void writeCommentAfterValueOnSameLine( const Value &root );
      bool hasCommentForValue( const Value &value );
      static std::string normalizeEOL( const std::string &text );

      typedef std::vector<std::string> ChildValues;

      ChildValues childValues_;
      std::string document_;
      std::string indentString_;
      int rightMargin_;
      int indentSize_;
      bool addChildValues_;
   };

   /** \brief Writes a Value in <a HREF="http://www.json.org">JSON</a> format in a human friendly way,
        to a stream rather than to a string.
    *
    * The rules for line break and indent are as follow:
    * - Object value:
    *     - if empty then print {} without indent and line break
    *     - if not empty the print '{', line break & indent, print one value per line
    *       and then unindent and line break and print '}'.
    * - Array value:
    *     - if empty then print [] without indent and line break
    *     - if the array contains no object value, empty array or some other value types,
    *       and all the values fit on one lines, then print the array on a single line.
    *     - otherwise, it the values do not fit on one line, or the array contains
    *       object or non empty array, then print one value per line.
    *
    * If the Value have comments then they are outputed according to their #CommentPlacement.
    *
    * \param indentation Each level will be indented by this amount extra.
    * \sa Reader, Value, Value::setComment()
    */
   class JSON_API StyledStreamWriter
   {
   public:
      StyledStreamWriter( std::string indentation="\t" );
      ~StyledStreamWriter(){}

   public:
      /** \brief Serialize a Value in <a HREF="http://www.json.org">JSON</a> format.
       * \param out Stream to write to. (Can be ostringstream, e.g.)
       * \param root Value to serialize.
       * \note There is no point in deriving from Writer, since write() should not return a value.
       */
      void write( std::ostream &out, const Value &root );

   private:
      void writeValue( const Value &value );
      void writeArrayValue( const Value &value );
      bool isMultineArray( const Value &value );
      void pushValue( const std::string &value );
      void writeIndent();
      void writeWithIndent( const std::string &value );
      void indent();
      void unindent();
      void writeCommentBeforeValue( const Value &root );
      void writeCommentAfterValueOnSameLine( const Value &root );
      bool hasCommentForValue( const Value &value );
      static std::string normalizeEOL( const std::string &text );

      typedef std::vector<std::string> ChildValues;

      ChildValues childValues_;
      std::ostream* document_;
      std::string indentString_;
      int rightMargin_;
      std::string indentation_;
      bool addChildValues_;
   };

   std::string JSON_API valueToString( Value::Int value );
   std::string JSON_API valueToString( Value::UInt value );
   std::string JSON_API valueToString( double value );
   std::string JSON_API valueToString( bool value );
   std::string JSON_API valueToQuotedString( const char *value );

   /// \brief Output using the StyledStreamWriter.
   /// \see Json::operator>>()
   std::ostream& operator<<( std::ostream&, const Value &root );

} // namespace Json



#endif // JSON_WRITER_H_INCLUDED
// END   include/json/writer.h

#endif // JSON_JSON_H_INCLUDED
// END   include/json/json.h