Name:       efl-assist
Summary:    EFL assist library
Version:    0.1.143
Release:    1
Group:      System/Libraries
License:    Flora-1.1
URL:        http://www.tizen.org/
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(tts)
BuildRequires:  pkgconfig(cairo)
BuildRequires:  pkgconfig(native-buffer)
BuildRequires:  pkgconfig(capi-media-image-util)
BuildRequires:  gettext
BuildRequires:  cmake
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig


%description
EFL assist library


%package devel
Summary:    EFL assista library (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   capi-base-common-devel


%description devel
EFL assist library providing small utility functions (devel)


%prep
%setup -q


%build
export CFLAGS+=" -fvisibility=hidden -fPIC -Wall"
export LDFLAGS+=" -fvisibility=hidden -Wl,-z,defs -Wl,--hash-style=both -Wl,--as-needed"

cmake \
	. -DCMAKE_INSTALL_PREFIX=/usr

make %{?jobs:-j%jobs}


%install
%make_install

mkdir -p %{buildroot}/%{_datadir}/license
cp %{_builddir}/%{buildsubdir}/LICENSE %{buildroot}/%{_datadir}/license/%{name}


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%{_libdir}/libefl-assist.so.*
%{_datadir}/locale/*
%{_datadir}/license/%{name}
%manifest %{name}.manifest

%files devel
%defattr(-,root,root,-)
%{_includedir}/efl-assist/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/efl-assist.pc
