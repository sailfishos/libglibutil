Name: libglibutil

Version: 1.0.80
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

# make_build macro appeared in rpm 4.12
%{!?make_build: %define make_build make %{_smp_mflags}}

# openSUSE workaround
%if 0%{?suse_version} > 0
%define libname %{name}%{so_ver}
%define so_ver %(echo %{version} | cut -d. -f1)
%description
Provides glib utility functions and macros

%package -n %{libname}
Summary: Runtime library for %{name}
%else
%define libname %{name}
%endif

Requires: glib2 >= %{glib_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description -n %{libname}
Provides glib utility functions and macros

%package devel
Summary: Development library for %{name}
Requires: %{libname} = %{version}
Requires: pkgconfig(glib-2.0) >= %{glib_version}

%description devel
This package contains the development library for %{name}.

%prep
%setup -q

%build
%make_build LIBDIR=%{_libdir} KEEP_SYMBOLS=1 release pkgconfig

%install
make LIBDIR=%{_libdir} DESTDIR=%{buildroot} install-dev

%check
make -C test test

%post -n %{libname} -p /sbin/ldconfig

%postun -n %{libname} -p /sbin/ldconfig

%files -n %{libname}
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
