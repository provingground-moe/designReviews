#if !defined(MEASUREMENT_H)
#define MEASUREMENT_H 1

#include <map>
#include "boost/make_shared.hpp"

#include "Peak.h"
#include "Schema.h"

/************************************************************************************************************/
/*
 * This is a base class for measurements of a set of quantities.  For example, we'll inherit from this
 * class to support Photometric measurements of various kinds.
 *
 * Measurement is a Go4 "Composite", able to hold individual measurements of type T (e.g. a PSF magnitude and
 * its error) or a set of measurements (PSF, Aperture, and model magnitudes, each with their errors and maybe
 * ancillary numbers such as a Sersic index or axis ratio).
 */
template<typename T>
class Measurement {
    typedef boost::shared_ptr<T> TPtr;
public:
    typedef boost::shared_ptr<Measurement> Ptr;
    typedef typename std::vector<TPtr>::iterator iterator;
    typedef typename std::vector<TPtr>::const_iterator const_iterator;

    Measurement() : _measuredValues(), _mySchema(new Schema) { }
    virtual ~Measurement() {}

    /// Return an iterator to the start of the set
    iterator begin() {
        return _measuredValues.begin();
    }
    /// Return a const iterator to the start of the set
    const_iterator begin() const {
        return _measuredValues.begin();
    }
    /// Return an iterator to the end of the set
    iterator end() {
        return _measuredValues.end();
    }
    /// Return an const iterator to the end of the set
    const_iterator end() const {
        return _measuredValues.end();
    }
    /// Add a (shared_pointer to) an individual measurement of type T
    void add(TPtr val) {
        _measuredValues.push_back(val);
    }

    /// Print all the values to os;  note that this is a virtual function called by operator<<
    virtual std::ostream &output(std::ostream &os) const {
        for (typename Measurement::const_iterator ptr = begin(); ptr != end(); ++ptr) {
            if (ptr != begin()) {
                os << " ";
            }
            os << "[" << **ptr << "]";
        }

        return os;
    }

    /// Return our Measurement's schema
    virtual Schema::Ptr getSchema() const {
        return _mySchema;
    }
    /// Resize the list of individual measurements of type T
    void resize(int n) { _data.resize(n); }

    /// Allocate enough space in _data to hold all values declared in the schema
    void init() {
        defineSchema(_mySchema);
        resize(getSchema()->size());    // getSchema() is virtual, but this is called from most-derived ctor
    }
    /**
     * Return some Measurement's value as a double given its Schema
     *
     * \sa getAsLong() to return as a long
     */
    double get(Schema const& se        ///< The schema entry for the value you want
              ) const {
        return getAsType<double>(se);
    }
    /**
     * Return some Measurement's value as a double given its Schema
     *
     * \sa getAsLong() to return as a long
     */
    double get(unsigned int i,                 ///< Index to set
               Schema const& se        ///< The schema entry for the value you want
              ) const {
        return getAsType<double>(i, se);
    }
    /**
     * Return some Measurement's value as a long given its Schema
     *
     * \sa get() to return as a double
     */
    long getAsLong(Schema const& se   ///< The schema entry for the value you want
                  ) const {
        return getAsType<long>(se);
    }
    /**
     * Return some Measurement's value as a long given its Schema
     *
     * \sa get() to return as a double
     */
    long getAsLong(unsigned int i,      ///< Index to set
                   Schema const& se     ///< The schema entry for the value you want
                  ) const {
        return getAsType<long>(i, se);
    }
    /**
     * Return a T by name and component
     *
     * Note that all values are returned as double; use the (protected) templated form of get if you need more
     * control
     */
    double get(std::string const& name,        ///< the name within T
               std::string const& component="" ///< the name within the set of measurements
              ) const {
        return get(getSchema()->find(name, component));
    }

    /**
     * Return an element T of an array by index name and component
     *
     * Note that all values are returned as double; use the (protected) templated form of get if you need more
     * control
     */
    double get(unsigned int i,                 ///< Index to set
               std::string const& name,        ///< the name within T
               std::string const& component="" ///< the name within the set of measurements
              ) const {
        return get(i, getSchema()->find(name, component));
    }             
protected:
    /// Fast compile-time-computed access to set the values of _data
    template<unsigned int INDEX, typename U>
    void set(U value                    ///< Desired value
            ) {
        assert(INDEX < _data.size());
        _data[INDEX] = value;
    }

    /// Fast compile-time-computed access to set the values of _data
    template<unsigned int INDEX, typename U>
    void set(unsigned int i,            ///< Index to set
             U value                    ///< Desired value
            ) {
        assert(INDEX + i < _data.size());
        _data[INDEX + i] = value;
    }

    /// Fast compile-time-computed access to retrieve the values of _data
    template<unsigned int INDEX, typename U>
    U get() const {
        assert(INDEX < _data.size());
        return boost::any_cast<U>(_data[INDEX]);
    }

    /// Fast compile-time-computed access to retrieve the values of _data as an array
    template<unsigned int INDEX, typename U>
    U get(unsigned int i                ///< Desired index
         ) const {
        assert(INDEX + i < _data.size());
        return boost::any_cast<U>(_data[INDEX + i]);
    }

private:
    virtual void defineSchema(Schema::Ptr ) {}

    /// Return a value as the specified type
    template<typename U>
    U getAsType(Schema const& se        ///< The schema entry for the value you want
               ) const {
        return getAsType<U>(0, se);
    }

    /// Return a value as the specified type
    template<typename U>
    U getAsType(unsigned int i,         ///< Index into array (if se is an array)
                Schema const& se        ///< The schema entry for the value you want
               ) const {
        unsigned int const index = se.getIndex() + i;
        if (index >= _data.size()) {
            std::ostringstream msg;
            if (index - i < _data.size()) { // the problem is that i takes us out of range
                msg << "Index " << i << " is out of range for " << se.getName() <<
                    "[0," << se.getDimen() - 1 << "]";
            } else {
                msg << "Index " << index << " out of range [0," << _data.size() << "] for " << se.getName();
            }
            throw std::runtime_error(msg.str());
        }
        boost::any const& val = _data[index];

        switch (se.getType()) {
          case Schema::CHAR:
            return static_cast<U>(boost::any_cast<char>(val));
          case Schema::SHORT:
            return static_cast<U>(boost::any_cast<short>(val));
          case Schema::INT:
            return static_cast<U>(boost::any_cast<int>(val));
          case Schema::LONG:
            return static_cast<U>(boost::any_cast<long>(val));
          case Schema::FLOAT:
            return static_cast<U>(boost::any_cast<float>(val));
          case Schema::DOUBLE:
            return static_cast<U>(boost::any_cast<double>(val));
          default:
            break;
        }
        
        std::ostringstream msg;
        msg << "Unable to retrieve value of type " << se.getType() << " for " << se.getName();
        throw std::runtime_error(msg.str());
    }

    typedef std::vector<boost::any> DataStore;
    // The elements of T (if a leaf)
    DataStore _data;

    // The set of Ts (if a composite)
    std::vector<TPtr> _measuredValues;

    // T's schema
    Schema::Ptr _mySchema;
};

/// Print v to os, using dynamic dispatch
template<typename T>
std::ostream &operator<<(std::ostream &os, Measurement<T> const& v) {
    return v.output(os);
}

/************************************************************************************************************/
/*
 * Measure a quantity using a set of algorithms.  Each algorithm will fill one item in the returned
 * Values (a Measurement)
 */
template<typename T, typename ImageT>
class MeasureQuantity {
private:
    typedef std::map<std::string, MeasureQuantity *> AlgorithmList;
public:
    typedef Measurement<typename T::element_type> Values;
    typedef MeasureQuantity *(*makeMeasureQuantityFunc)();

    MeasureQuantity() : _algorithms() {}
    virtual ~MeasureQuantity() {}

    /// Include the algorithm called name in the list of measurement algorithms to use
    ///
    /// This name is looked up in the registry (\sa declare), and used as the name of the
    /// measurement if you wish to retrieve it using the schema
    ///
    void addAlgorithm(std::string const& name ///< The name of the algorithm
                     ) {
        _algorithms[name] = _lookupAlgorithm(name)();
    }
    /// Actually measure im using all requested algorithms, returning the result
    Values measure(ImageT const& im,    ///< the Image (or other object) to process
                   Peak const& peak     ///< approximate position of object's centre
                  ) {
        Values values;

        for (typename AlgorithmList::iterator ptr = _algorithms.begin(); ptr != _algorithms.end(); ++ptr) {
            T val = ptr->second->doMeasure(im, peak);
            val->getSchema()->setComponent(ptr->first); // name this type of measurement (e.g. psf)
            values.add(val);
        }

        return values;
    }

    static bool declare(std::string const& name, makeMeasureQuantityFunc func);
private:
    //
    // The list of algorithms that we wish to use
    //
    AlgorithmList _algorithms;
    //
    // A mapping from names to algorithms
    //
    // _registryWorker must be inline as it contains a critical static variable, _registry
    //    
    typedef std::map<std::string, makeMeasureQuantityFunc> AlgorithmRegistry;

    static inline makeMeasureQuantityFunc _registryWorker(std::string const& name,
                                                          makeMeasureQuantityFunc func);
    static makeMeasureQuantityFunc _lookupAlgorithm(std::string const& name);
    /// The unknown algorithm; used to allow _lookupAlgorithm use _registryWorker
    static MeasureQuantity *_iefbr14() { return NULL; }
    //
    // Do the real work of measuring things
    //
    // Can't be pure virtual as we create a do-nothing MeasureQuantity which we then add to
    //
    virtual T doMeasure(ImageT const&, Peak const&) {
        return T();
    }
};

/**
 * Support the algorithm registry
 */
template<typename T, typename ImageT>
typename MeasureQuantity<T, ImageT>::makeMeasureQuantityFunc MeasureQuantity<T, ImageT>::_registryWorker(
        std::string const& name,
        typename MeasureQuantity<T, ImageT>::makeMeasureQuantityFunc func
                                                                                                        )
{
    static typename MeasureQuantity<T, ImageT>::AlgorithmRegistry _registry;

    if (func == _iefbr14) {     // lookup func
        typename MeasureQuantity<T, ImageT>::AlgorithmRegistry::const_iterator ptr = _registry.find(name);
        if (ptr == _registry.end()) {
            throw std::runtime_error("Unknown algorithm " + name);
        }
        
        func = ptr->second;
    } else {                            // register func
        _registry[name] = func;
    }

    return func;
}

/**
 * Register the factory function for a named algorithm
 */
template<typename T, typename ImageT>
bool MeasureQuantity<T, ImageT>::declare(
        std::string const& name,
        typename MeasureQuantity<T, ImageT>::makeMeasureQuantityFunc func
                                        )
{
    _registryWorker(name, func);

    return true;
}

/**
 * Return the factory function for a named algorithm
 */
template<typename T, typename ImageT>
typename MeasureQuantity<T, ImageT>::makeMeasureQuantityFunc
MeasureQuantity<T, ImageT>::_lookupAlgorithm(std::string const& name)
{
    return _registryWorker(name, _iefbr14);
}

/**
 * Provide the boilerplate required to support measuring quantities using a variety of algorithms
 *
 * Arguments are:
 *  @param QUANTITY         Class that is to be measured; e.g. Photometry
 *  @param NAME             Name of algorithm; e.g. "model"
 *  @param ALG              Name of class is ALG##QUANTITY; e.g. Model
 *  @param DO_MEASURE_ARGS  Arguments of the doMeasure member, including parens; e.g. (Image const& im)
 */
#define MEASUREMENT_BOILERPLATE(QUANTITY, NAME, ALG, DO_MEASURE_ARGS) \
/** */ \
/* Measure ALG##QUANTITY.  The real work is done by the doMeasure function */ \
/* */ \
class ALG##Measure##QUANTITY : public Measure##QUANTITY { \
public: \
    ALG##Measure##QUANTITY() :  Measure##QUANTITY() {} \
protected: \
    virtual QUANTITY::Ptr doMeasure DO_MEASURE_ARGS; \
}; \
 \
/** */ \
/* Wrap the constructor so we can register it by name */ \
/* */ \
Measure##QUANTITY::FactoryPtr make##ALG##Measure##QUANTITY() { \
    return new ALG##Measure##QUANTITY(); \
} \
 \
/** */ \
/* Declare the existence of an "NAME" algorithm */ \
/* */ \
namespace { \
    volatile bool isInstance[] = { \
        Measure##QUANTITY::declare(NAME, make##ALG##Measure##QUANTITY), \
    }; \
} \

#endif
