Name: libglibutil
Version: 1.0.35
Release: 0
Summary: Library of glib utilities
Group: Development/Libraries
License: BSD
URL: https://git.merproject.org/mer-core/libglibutil
Source: %{name}-%{version}.tar.bz2
BuildRequires: pkgconfig(glib-2.0)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Provides glib utility functions and macros

%package devel
Summary: Development library for %{name}
Requires: %{name} = %{version}
Requires: pkgconfig

%description devel
This package contains the development library for %{name}.

%prep
%setup -q

%build
make KEEP_SYMBOLS=1 release pkgconfig

%install
rm -rf %{buildroot}
make install-dev DESTDIR=%{buildroot}

%check
make -C test test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/%{name}.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/*.pc
%{_libdir}/%{name}.so
%{_includedir}/gutil/*.h
