Name: libglibutil

Version: 1.0.72
Release: 0
Summary: Library of glib utilities
License: BSD
URL: https://github.com/sailfishos/libglibutil
Source: %{name}-%{version}.tar.bz2

%define glib_version 2.32

BuildRequires: pkgconfig
BuildRequires: pkgconfig(glib-2.0) >= %{glib_version}

# license macro requires rpm >= 4.11
BuildRequires: pkgconfig(rpm)
%define license_support %(pkg-config --exists 'rpm >= 4.11'; echo $?)

Requires: glib2 >= %{glib_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Provides glib utility functions and macros

%package devel
Summary: Development library for %{name}
Requires: %{name} = %{version}
Requires: pkgconfig(glib-2.0) >= %{glib_version}

%description devel
This package contains the development library for %{name}.

%prep
%setup -q

%build
make %{_smp_mflags} LIBDIR=%{_libdir} KEEP_SYMBOLS=1 release pkgconfig

%install
make LIBDIR=%{_libdir} DESTDIR=%{buildroot} install-dev

%check
make -C test test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/%{name}.so.*
%if %{license_support} == 0
%license LICENSE
%endif

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/gutil
%{_libdir}/pkgconfig/*.pc
%{_libdir}/%{name}.so
%{_includedir}/gutil/*.h
