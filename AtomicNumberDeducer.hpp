#if !defined(ATOMIC_NUMBER_DEDUCER_HPP)
#define ATOMIC_NUMBER_DEDUCER_HPP

#include <vector>


namespace loos {

  namespace internal {

    class AtomicNumberDeducer {
      typedef std::pair<double, unsigned int>   MassNumber;
    public:
      AtomicNumberDeducer() {
        initialize();
      }

      unsigned int deduceFromMass(const double mass, const double tolerance);


    private:
      void initialize();
      std::vector<MassNumber> element_table;
    };

  };

  unsigned int deduceAtomicNumberFromMass(const double mass, const double tolerance = 0.1);

};




#endif