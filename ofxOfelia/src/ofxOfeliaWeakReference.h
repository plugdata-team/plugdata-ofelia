#pragma once
#include <functional>
#include <mutex>
#include <atomic>
#include <assert.h>

#define DECLARE_NON_COPYABLE(className) \
    className (const className&) = delete;\
    className& operator= (const className&) = delete;

#define DECLARE_WEAK_REFERENCEABLE(Class) \
    struct WeakRefMaster  : public ofxOfeliaWeakReference<Class>::Master { ~WeakRefMaster() { this->clear(); } }; \
    WeakRefMaster masterReference; \
    friend class ofxOfeliaWeakReference<Class>; \

// Some JUCE classes that help us to decide whether or not the object has been deallocated
// Useful because we have a lot of async callbacks!


class ofxOfeliaReferenceCountedObject
{
public:
    //==============================================================================
    /** Increments the object's reference count.

        This is done automatically by the smart pointer, but is public just
        in case it's needed for nefarious purposes.
    */
    void incReferenceCount() noexcept
    {
        ++refCount;
    }

    /** Decreases the object's reference count.
        If the count gets to zero, the object will be deleted.
    */
    void decReferenceCount() noexcept
    {
        assert (getReferenceCount() > 0);

        if (--refCount == 0)
            delete this;
    }

    /** Decreases the object's reference count.
        If the count gets to zero, the object will not be deleted, but this method
        will return true, allowing the caller to take care of deletion.
    */
    bool decReferenceCountWithoutDeleting() noexcept
    {
        assert (getReferenceCount() > 0);
        return --refCount == 0;
    }

    /** Returns the object's current reference count. */
    int getReferenceCount() const noexcept       { return refCount.load(); }


protected:
    //==============================================================================
    /** Creates the reference-counted object (with an initial ref count of zero). */
    ofxOfeliaReferenceCountedObject() = default;

    /** Copying from another object does not affect this one's reference-count. */
    ofxOfeliaReferenceCountedObject (const ofxOfeliaReferenceCountedObject&) noexcept {}
    /** Copying from another object does not affect this one's reference-count. */
    ofxOfeliaReferenceCountedObject (ofxOfeliaReferenceCountedObject&&) noexcept {}
    /** Copying from another object does not affect this one's reference-count. */
    ofxOfeliaReferenceCountedObject& operator= (const ofxOfeliaReferenceCountedObject&) noexcept  { return *this; }
    /** Copying from another object does not affect this one's reference-count. */
    ofxOfeliaReferenceCountedObject& operator= (ofxOfeliaReferenceCountedObject&&) noexcept       { return *this; }

    /** Destructor. */
    virtual ~ofxOfeliaReferenceCountedObject()
    {
        // it's dangerous to delete an object that's still referenced by something else!
        assert (getReferenceCount() == 0);
    }

    /** Resets the reference count to zero without deleting the object.
        You should probably never need to use this!
    */
    void resetReferenceCount() noexcept
    {
        refCount = 0;
    }

private:
    //==============================================================================
    std::atomic<int> refCount { 0 };
};

template <class ObjectType>
class ofxOfeliaReferenceCountedObjectPtr
{
public:
    /** The class being referenced by this pointer. */
    using ReferencedType = ObjectType;

    //==============================================================================
    /** Creates a pointer to a null object. */
    ofxOfeliaReferenceCountedObjectPtr() = default;

    /** Creates a pointer to a null object. */
    ofxOfeliaReferenceCountedObjectPtr (decltype (nullptr)) noexcept {}

    /** Creates a pointer to an object.
        This will increment the object's reference-count.
    */
    ofxOfeliaReferenceCountedObjectPtr (ReferencedType* refCountedObject) noexcept
        : referencedObject (refCountedObject)
    {
        incIfNotNull (refCountedObject);
    }

    /** Creates a pointer to an object.
        This will increment the object's reference-count.
    */
    ofxOfeliaReferenceCountedObjectPtr (ReferencedType& refCountedObject) noexcept
        : referencedObject (&refCountedObject)
    {
        refCountedObject.incReferenceCount();
    }

    /** Copies another pointer.
        This will increment the object's reference-count.
    */
    ofxOfeliaReferenceCountedObjectPtr (const ofxOfeliaReferenceCountedObjectPtr& other) noexcept
        : referencedObject (other.referencedObject)
    {
        incIfNotNull (referencedObject);
    }

    /** Takes-over the object from another pointer. */
    ofxOfeliaReferenceCountedObjectPtr (ofxOfeliaReferenceCountedObjectPtr&& other) noexcept
        : referencedObject (other.referencedObject)
    {
        other.referencedObject = nullptr;
    }

    /** Copies another pointer.
        This will increment the object's reference-count (if it is non-null).
    */
    template <typename Convertible>
    ofxOfeliaReferenceCountedObjectPtr (const ofxOfeliaReferenceCountedObjectPtr<Convertible>& other) noexcept
        : referencedObject (other.get())
    {
        incIfNotNull (referencedObject);
    }

    /** Changes this pointer to point at a different object.
        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    ofxOfeliaReferenceCountedObjectPtr& operator= (const ofxOfeliaReferenceCountedObjectPtr& other)
    {
        return operator= (other.referencedObject);
    }

    /** Changes this pointer to point at a different object.
        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    template <typename Convertible>
    ofxOfeliaReferenceCountedObjectPtr& operator= (const ofxOfeliaReferenceCountedObjectPtr<Convertible>& other)
    {
        return operator= (other.get());
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    ofxOfeliaReferenceCountedObjectPtr& operator= (ReferencedType* newObject)
    {
        if (newObject != nullptr)
            return operator= (*newObject);

        reset();
        return *this;
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    ofxOfeliaReferenceCountedObjectPtr& operator= (ReferencedType& newObject)
    {
        if (referencedObject != &newObject)
        {
            newObject.incReferenceCount();
            auto* oldObject = referencedObject;
            referencedObject = &newObject;
            decIfNotNull (oldObject);
        }

        return *this;
    }

    /** Resets this pointer to a null pointer. */
    ofxOfeliaReferenceCountedObjectPtr& operator= (decltype (nullptr))
    {
        reset();
        return *this;
    }

    /** Takes-over the object from another pointer. */
    ofxOfeliaReferenceCountedObjectPtr& operator= (ofxOfeliaReferenceCountedObjectPtr&& other) noexcept
    {
        std::swap (referencedObject, other.referencedObject);
        return *this;
    }

    /** Destructor.
        This will decrement the object's reference-count, which will cause the
        object to be deleted when the ref-count hits zero.
    */
    ~ofxOfeliaReferenceCountedObjectPtr()
    {
        decIfNotNull (referencedObject);
    }

    //==============================================================================
    /** Returns the object that this pointer references.
        The pointer returned may be null, of course.
    */
    ReferencedType* get() const noexcept                    { return referencedObject; }

    /** Resets this object to a null pointer. */
    void reset() noexcept
    {
        auto oldObject = referencedObject;  // need to null the pointer before deleting the object
        referencedObject = nullptr;         // in case this ptr is itself deleted as a side-effect
        decIfNotNull (oldObject);           // of the destructor
    }

    // the -> operator is called on the referenced object
    ReferencedType* operator->() const noexcept
    {
        assert (referencedObject != nullptr); // null pointer method call!
        return referencedObject;
    }

    /** Dereferences the object that this pointer references.
        The pointer returned may be null, of course.
    */
    ReferencedType& operator*() const noexcept              { assert (referencedObject != nullptr); return *referencedObject; }

    /** Checks whether this pointer is null */
    bool operator== (decltype (nullptr)) const noexcept     { return referencedObject == nullptr; }
    /** Checks whether this pointer is null */
    bool operator!= (decltype (nullptr)) const noexcept     { return referencedObject != nullptr; }

    /** Compares two ofxOfeliaReferenceCountedObjectPtrs. */
    bool operator== (const ObjectType* other) const noexcept                 { return referencedObject == other; }
    /** Compares two ofxOfeliaReferenceCountedObjectPtrs. */
    bool operator== (const ofxOfeliaReferenceCountedObjectPtr& other) const noexcept  { return referencedObject == other.get(); }
    /** Compares two ofxOfeliaReferenceCountedObjectPtrs. */
    bool operator!= (const ObjectType* other) const noexcept                 { return referencedObject != other; }
    /** Compares two ofxOfeliaReferenceCountedObjectPtrs. */
    bool operator!= (const ofxOfeliaReferenceCountedObjectPtr& other) const noexcept  { return referencedObject != other.get(); }

    /** Checks whether this pointer is null */
    explicit operator bool() const noexcept                 { return referencedObject != nullptr; }

private:
    //==============================================================================
    ReferencedType* referencedObject = nullptr;

    static void incIfNotNull (ReferencedType* o) noexcept
    {
        if (o != nullptr)
            o->incReferenceCount();
    }

    static void decIfNotNull (ReferencedType* o) noexcept
    {
        
        struct ContainerDeletePolicy
        {
            static void destroy (ObjectType* object)
            {
                // If the line below triggers a compiler error, it means that you are using
                // an incomplete type for ObjectType (for example, a type that is declared
                // but not defined). This is a problem because then the following delete is
                // undefined behaviour. The purpose of the sizeof is to capture this situation.
                // If this was caused by a OwnedArray of a forward-declared type, move the
                // implementation of all methods trying to use the OwnedArray (e.g. the destructor
                // of the class owning it) into cpp files where they can see to the definition
                // of ObjectType. This should fix the error.
                [[maybe_unused]] constexpr auto size = sizeof (ObjectType);

                delete object;
            }
        };
        
        if (o != nullptr && o->decReferenceCountWithoutDeleting())
            ContainerDeletePolicy::destroy (o);
    }
};


template <class ObjectType, class ReferenceCountingType = ofxOfeliaReferenceCountedObject>
class ofxOfeliaWeakReference
{
public:
    /** Creates a null ofxOfeliaWeakReference. */
    inline ofxOfeliaWeakReference() = default;

    /** Creates a ofxOfeliaWeakReference that points at the given object. */
    ofxOfeliaWeakReference (ObjectType* object)  : holder (getRef (object)) {}

    /** Creates a copy of another ofxOfeliaWeakReference. */
    ofxOfeliaWeakReference (const ofxOfeliaWeakReference& other) noexcept         : holder (other.holder) {}

    /** Move constructor */
    ofxOfeliaWeakReference (ofxOfeliaWeakReference&& other) noexcept              : holder (std::move (other.holder)) {}

    /** Copies another pointer to this one. */
    ofxOfeliaWeakReference& operator= (const ofxOfeliaWeakReference& other)       { holder = other.holder; return *this; }

    /** Copies another pointer to this one. */
    ofxOfeliaWeakReference& operator= (ObjectType* newObject)            { holder = getRef (newObject); return *this; }

    /** Move assignment operator */
    ofxOfeliaWeakReference& operator= (ofxOfeliaWeakReference&& other) noexcept   { holder = std::move (other.holder); return *this; }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    ObjectType* get() const noexcept                            { return holder != nullptr ? holder->get() : nullptr; }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    operator ObjectType*() const noexcept                       { return get(); }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    ObjectType* operator->() const noexcept                     { return get(); }

    /** This returns true if this reference has been pointing at an object, but that object has
        since been deleted.

        If this reference was only ever pointing at a null pointer, this will return false. Using
        operator=() to make this refer to a different object will reset this flag to match the status
        of the reference from which you're copying.
    */
    bool wasObjectDeleted() const noexcept                      { return holder != nullptr && holder->get() == nullptr; }

    //==============================================================================
    /** This class is used internally by the ofxOfeliaWeakReference class - don't use it directly
        in your code!
        @see ofxOfeliaWeakReference
    */
    class SharedPointer   : public ReferenceCountingType
    {
    public:
        explicit SharedPointer (ObjectType* obj) noexcept : owner (obj) {}

        inline ObjectType* get() const noexcept     { return owner; }
        void clearPointer() noexcept                { owner = nullptr; }

    private:
        ObjectType* owner;

        DECLARE_NON_COPYABLE (SharedPointer)
    };

    using SharedRef = ofxOfeliaReferenceCountedObjectPtr<SharedPointer>;

    //==============================================================================
    /**
        This class is embedded inside an object to which you want to attach ofxOfeliaWeakReference pointers.
        See the ofxOfeliaWeakReference class notes for an example of how to use this class.
        @see ofxOfeliaWeakReference
    */
    class Master
    {
    public:
        Master() = default;

        ~Master() noexcept
        {
            // You must remember to call clear() in your source object's destructor! See the notes
            // for the ofxOfeliaWeakReference class for an example of how to do this.
            assert (sharedPointer == nullptr || sharedPointer->get() == nullptr);
        }

        /** The first call to this method will create an internal object that is shared by all weak
            references to the object.
        */
        SharedRef getSharedPointer (ObjectType* object)
        {
            if (sharedPointer == nullptr)
            {
                sharedPointer = *new SharedPointer (object);
            }
            else
            {
                // You're trying to create a weak reference to an object that has already been deleted!!
                assert (sharedPointer->get() != nullptr);
            }

            return sharedPointer;
        }

        /** The object that owns this master pointer should call this before it gets destroyed,
            to zero all the references to this object that may be out there. See the ofxOfeliaWeakReference
            class notes for an example of how to do this.
        */
        void clear() noexcept
        {
            if (sharedPointer != nullptr)
                sharedPointer->clearPointer();
        }

        /** Returns the number of ofxOfeliaWeakReferences that are out there pointing to this object. */
        int getNumActiveofxOfeliaWeakReferences() const noexcept
        {
            return sharedPointer == nullptr ? 0 : (sharedPointer->getReferenceCount() - 1);
        }

    private:
        SharedRef sharedPointer;

        DECLARE_NON_COPYABLE (Master)
    };

private:
    SharedRef holder;

    static SharedRef getRef (ObjectType* o)
    {
        if (o != nullptr)
            return o->masterReference.getSharedPointer (o);

        return {};
    }
};
