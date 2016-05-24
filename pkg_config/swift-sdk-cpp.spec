%global version {{version}}
%define debug_package %{nil}

Name:		swift-sdk-cpp
Version:	%{version}
Release:	{{build}}%{?dist}
Summary:	Swift SDK for C++
Group:		Applications/File
License:	MIT
URL:		https://github.com/bshafiee/Swift_CPP_SDK
Source0:	swift-sdk-cpp-%{version}.orig.tar.gz

BuildRequires: gcc-c++ >= 4.9.0,
BuildRequires: git,
BuildRequires: poco-devel,
BuildRequires: poco-foundation,
BuildRequires: poco-netssl,
BuildRequires: poco-util,
BuildRequires: poco-xml,
BuildRequires: make

%description
Swift SDK for C++

%prep
%setup -q

%build
make

%install
make install DESTDIR=%{buildroot}
find %{buildroot}

%files
%{_prefix}/lib/*
%{_includedir}/*

%doc

%changelog

