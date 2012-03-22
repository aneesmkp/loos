
%header %{
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <Coord.hpp>
%}

%wrapper %{

  typedef loos::Coord<double>   GCoord;

  %}


namespace loos {

  template<class T>
  class Coord {
    enum { X=0, Y=1, Z=2, MAXCOORD } CoordIndex;

    //! The threshold for vector equality
    static const double epsilon = 1e-16;

  public:



    // Constructors

    Coord();
    Coord(const T ax, const T ay, const T az);
    Coord(const Coord<T>& o);
    Coord(const T x);

    void x(const T ax);
    T x() const;
    void y(const T ay);
    T y() const;
    void z(const T az);
    T z() const;
    void set(const T x, const T y, const T z);
    Coord<T>& operator+=(const Coord<T>& rhs);
    Coord<T> operator+(const Coord<T>& rhs) const;
    Coord<T>& operator-=(const Coord<T>& rhs);
    Coord<T> operator-(const Coord<T>& rhs) const;
    Coord<T> operator-();
    Coord<T>& operator*=(const T rhs);
    Coord<T> operator*(const T rhs) const;
    Coord<T>& operator/=(const T rhs);
    Coord<T> operator/(const T rhs) const;
    T dot(const Coord<T>& rhs) const;
    T operator*(const Coord<T>rhs) const;
    Coord<T> cross(const Coord<T>& rhs) const;
    Coord<T>& operator^=(const Coord<T>& rhs);
    Coord<T> operator^(const Coord<T>& rhs) const;
    Coord<T>& operator%=(const Coord<T>& rhs);
    Coord<T> operator%(const Coord<T>& rhs) const;
    void reimage(const Coord<T>& box);
    T length2(void) const;
    T length(void) const;
    T distance2(const Coord<T>& o) const;
    T distance2(const Coord<T>& o, const Coord<T>& box) const;
    T distance(const Coord<T>& o) const;
    T distance(const Coord<T>& o, const Coord<T>& box) const;
    void zero(void);
    bool operator==(const Coord<T>& rhs) const;
    bool operator!=(const Coord<T>& rhs) const;
  };



  %extend Coord<T> {
    T __getitem__(const int i) {
      if (i < 0 || i >= 3)
        return(0);
      return((*$self)[i]);
    }
    
    void __setitem__(const int i, const T d) {
      if (3 >= i && i >= 0)
        (*$self)[i] = d;
    }

  };

};


%extend loos::Coord<double> {
  char* __str__() {
    static char buf[1024];
    std::ostringstream oss;
    oss << *$self;
    strncpy(buf, oss.str().c_str(), sizeof(buf));
    return(buf);
  }

 };

%template(GCoord)  loos::Coord<double>;

%rename(__add__)  loos::Coord<double>::operator+;
%rename(__sub__) loos::Coord<double>::operator-;
%rename(__mul__) loos::Coord<double>::operator*;
%rename(__div__) loos::Coord<double>::operator/;
%rename(__mod__) loos::Coord<double>::operator%;
%rename(__pow__) loos::Coord<double>::operator^;
%rename(__eq__) loos::Coord<double>::operator==;
%rename(__ne__) loos::Coord<double>::operator!=;



%template(GCoordVector)   std::vector<loos::Coord<double> >;