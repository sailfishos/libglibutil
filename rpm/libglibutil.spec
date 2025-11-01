# The main package is now named libglibutil1
Name:           libglibutil1
Version:        1.0.80
Release:        0
Summary:        Library of glib utilities
License:        BSD
URL:            https://github.com/sailfishos/libglibutil
# The Source tag must be hardcoded to the original tarball name
Source:         libglibutil-%{version}.tar.bz2

%define glib_version 2.32

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(glib-2.0) >= %{glib_version}

# make_build macro appeared in rpm 4.12
%{!?make_build: %define make_build make %{_smp_mflags}}

# Requirements are now in the main package
Requires:       glib2 >= %{glib_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

# Compatibility tags are now in the main package
Provides:       libglibutil = %{version}-%{release}
Provides:       libglibutil(%{_arch}) = %{version}-%{release}
Obsoletes:      libglibutil < %{version}

%description
Provides glib utility functions and macros

%package devel
Summary:        Development library for libglibutil
# This now correctly requires the main package, libglibutil1
Requires:       %{name} = %{version}
Requires:       pkgconfig(glib-2.0) >= %{glib_version}

%description devel
This package contains the development library for libglibutil.

%prep
# This -n flag is critical. It tells setup the directory name
# inside the tarball is "libglibutil-..."
%setup -q -n libglibutil-%{version}

%build
%make_build LIBDIR=%{_libdir} KEEP_SYMBOLS=1 release pkgconfig

%install
make LIBDIR=%{_libdir} DESTDIR=%{buildroot} install-dev

%check
make -C test test

# These scripts now apply to the main (libglibutil1) package
%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

# These files now apply to the main (libglibutil1) package
%files
%defattr(-,root,root,-)
%{_libdir}/libglibutil.so.*
# Removed the complex license logic that was failing
%license LICENSE

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/gutil
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libglibutil.so
%{_includedir}/gutil/*.h
