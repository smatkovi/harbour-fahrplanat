Name:       harbour-fahrplanat

Summary:    Fahrplanauskunft für Österreich (inoffizieller HAFAS-Client)
Version:    0.1.7
Release:    1
License:    GPLv3
URL:        https://github.com/smatkovi/harbour-fahrplanat
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  desktop-file-utils

%description
Verbindungssuche mit Echtzeitdaten, Zwischenhalten, Gleisangaben,
Verspätungen und Störungsmeldungen für den öffentlichen Verkehr in
Österreich. Die Daten kommen von der HAFAS-Schnittstelle unter
fahrplan.oebb.at. Inoffizieller Client, nicht von der ÖBB.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 VERSION=%{version}
%make_build

%install
%qmake5_install
desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
  %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
