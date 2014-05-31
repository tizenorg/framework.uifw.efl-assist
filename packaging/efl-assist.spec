Name:       efl-assist
Summary:    EFL assist library
Version:    0.1.66b05
Release:    1
Group:      System/Libraries
License:    APLv2
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(tts)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
%if %{_repository} == "mobile"
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-appfw-application)
%endif
%if %{_repository} == "wearable"
BuildRequires:  pkgconfig(cairo)
BuildRequires:  gettext
%endif




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
export CFLAGS+=" -fvisibility=hidden"
export LDFLAGS+=" -fvisibility=hidden"

cd  %{_repository} && cmake . -DCMAKE_INSTALL_PREFIX=/usr
make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
cd  %{_repository} && %make_install
mkdir -p %{buildroot}/usr/share/license
cp %{_builddir}/%{buildsubdir}/LICENSE %{buildroot}/usr/share/license/%{name}


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
#%{_bindir}/*
%{_libdir}/libefl-assist.so.*
%manifest %{name}.manifest
%if %{_repository} == "wearable"
/usr/share/locale/*
%endif
/usr/share/license/%{name}


%files devel
%defattr(-,root,root,-)
%{_includedir}/efl-assist/*.h
%{_libdir}/*.so
%{_libdir}/pkgconfig/efl-assist.pc

