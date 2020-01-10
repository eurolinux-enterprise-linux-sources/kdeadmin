
Name: kdeadmin
Summary: KDE Administrative tools
Epoch: 7
Version: 4.10.5
Release: 4%{?dist}

Group: User Interface/Desktops
License: GPLv2
URL: http://www.kde.org/
%global revision %(echo %{version} | cut -d. -f3)
%if %{revision} >= 50
%global stable unstable
%else
%global stable stable
%endif
Source0: http://download.kde.org/%{stable}/%{version}/src/%{name}-%{version}.tar.xz

# fix ksystemlog to find log files correctly
Patch1: kdeadmin-4.8.4-syslog.patch

BuildRequires: kdelibs4-devel >= %{version}
BuildRequires: kdepimlibs-devel >= %{version}

Requires: kdelibs4 >= %{version}
Requires: kdepimlibs >= %{version}

Obsoletes: kdeadmin-kpackage < %{epoch}:%{version}-%{release}

%description
The %{name} package includes administrative tools including:
* kcron: systemsettings module for the cron task scheduler
* ksystemlog: system log viewer
* kuser: user manager


%prep
%setup -q -n kdeadmin-%{version}

%patch1 -p1 -b .ksystemlog

%build

mkdir -p %{_target_platform}
pushd %{_target_platform}
%{cmake_kde4} ..
popd

make %{?_smp_mflags} -C %{_target_platform}


%install
make install/fast DESTDIR=%{buildroot} -C %{_target_platform}

# remove broken .pc file which has no business to be in a non-devel pkg anyway
rm -rf %{buildroot}%{_libdir}/pkgconfig


%post
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null || :

%posttrans
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
touch --no-create %{_kde4_iconsdir}/hicolor &> /dev/null || :
gtk-update-icon-cache %{_kde4_iconsdir}/hicolor &> /dev/null || :
fi

%files
%doc ksystemlog/COPYING
# ksystemlog
%{_kde4_bindir}/ksystemlog
%{_kde4_appsdir}/ksystemlog/
%{_kde4_datadir}/applications/kde4/ksystemlog.desktop
%{_kde4_docdir}/HTML/en/ksystemlog/
# kcm_cron
%{_kde4_datadir}/kde4/services/kcm_cron.desktop
%{_kde4_docdir}/HTML/en/kcron/
%{_kde4_libdir}/kde4/kcm_cron.so
# kuser
%{_kde4_bindir}/kuser
%{_kde4_appsdir}/kuser/
%{_kde4_datadir}/applications/kde4/kuser.desktop
%{_kde4_datadir}/config.kcfg/kuser.kcfg
%{_kde4_docdir}/HTML/en/kuser/
%{_kde4_iconsdir}/hicolor/*/*/kuser.*


%changelog
* Fri Sep 08 2017 Jan Grulich <jgrulich@redhat.com> - 7:4.10.5-4
- Enable kuser
  Resolves: bz#1471610

* Tue Jan 28 2014 Daniel Mach <dmach@redhat.com> - 7:4.10.5-3
- Mass rebuild 2014-01-24

* Fri Dec 27 2013 Daniel Mach <dmach@redhat.com> - 7:4.10.5-2
- Mass rebuild 2013-12-27

* Sun Jun 30 2013 Than Ngo <than@redhat.com> - 4.10.5-1
- 4.10.5

* Sat Jun 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 7:4.10.4-1
- 4.10.4

* Mon May 06 2013 Than Ngo <than@redhat.com> - 4.10.3-1
- 4.10.3

* Sun Mar 31 2013 Rex Dieter <rdieter@fedoraproject.org> - 7:4.10.2-1
- 4.10.2

* Sat Mar 02 2013 Rex Dieter <rdieter@fedoraproject.org> - 7:4.10.1-1
- 4.10.1

* Fri Feb 01 2013 Rex Dieter <rdieter@fedoraproject.org> - 7:4.10.0-1
- 4.10.0

* Tue Jan 22 2013 Rex Dieter <rdieter@fedoraproject.org> - 7:4.9.98-1
- 4.9.98

* Fri Jan 04 2013 Rex Dieter <rdieter@fedoraproject.org> - 7:4.9.97-1
- 4.9.97

* Thu Dec 20 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.9.95-1
- 4.9.95

* Thu Dec 13 2012 Kevin Kofler <Kevin@tigcc.ticalc.org> - 7:4.9.90-2
- don't Obsolete system-config-printer-kde, let kde-print-manager do that

* Tue Dec 04 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.9.90-1
- 4.9.90

* Mon Dec 03 2012 Than Ngo <than@redhat.com> - 7:4.9.4-1
- 4.9.4

* Sat Nov 03 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.9.3-1
- 4.9.3

* Tue Oct 09 2012 Than Ngo <than@redhat.com> - 7:4.9.2-2
- add fedora/rhel condition

* Sat Sep 29 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.9.2-1
- 4.9.2

* Mon Sep 03 2012 Than Ngo <than@redhat.com> - 7:4.9.1-1
- 4.9.1

* Sat Aug 04 2012 Than Ngo <than@redhat.com> - 7:4.9.0-2
- fix ksystemlog to find log files correctly
- mv s*c*p-kde docs in subpackage system-config-printer-kde

* Thu Jul 26 2012 Lukas Tinkl <ltinkl@redhat.com> - 7:4.9.0-1
- 4.9.0

* Thu Jul 19 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 7:4.8.97-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Thu Jul 12 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.8.97-1
- 4.8.97

* Wed Jun 27 2012 Jaroslav Reznik <jreznik@redhat.com> - 7:4.8.95-1
- 4.8.95

* Sat Jun 09 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.8.90-1
- 4.8.90

* Sat May 26 2012 Jaroslav Reznik <jreznik@redhat.com> - 7:4.8.80-1
- 4.8.80
- add system-config-printer-kde docs

* Mon Apr 30 2012 Jaroslav Reznik <jreznik@redhat.com> - 7:4.8.3-1
- 4.8.3

* Fri Mar 30 2012 Rex Dieter <rdieter@fedoraproject.org> - 7:4.8.2-1
- 4.8.2

* Tue Mar 06 2012 Radek Novacek <rnovacek@redhat.com> 7:4.8.1-1
- 4.8.1

* Fri Jan 20 2012 Jaroslav Reznik <jreznik@redhat.com> - 7:4.8.0-1
- 4.8.0

* Wed Jan 04 2012 Radek Novacek <rnovacek@redhat.com> - 7:4.7.97-1
- 4.7.97

* Wed Dec 21 2011 Radek Novacek <rnovacek@redhat.com> - 7:4.7.95-1
- 4.7.95

* Sun Dec 04 2011 Rex Dieter <rdieter@fedoraproject.org> - 7:4.7.90-1
- 4.7.90

* Sat Dec 03 2011 Rex Dieter <rdieter@fedoraproject.org> 7:4.7.80-2
- system-config-printer-kde: Requires: dbus-python PyQt4 python-cups

* Fri Nov 25 2011 Radek Novacek <rnovacek@redhat.com> 7:4.7.80-1
- 4.7.80 (beta 1)

* Sat Oct 29 2011 Rex Dieter <rdieter@fedoraproject.org> 7:4.7.3-1
- 4.7.3

* Tue Oct 04 2011 Rex Dieter <rdieter@fedoraproject.org> 7:4.7.2-1
- 4.7.2

* Mon Sep 19 2011 Kevin Kofler <Kevin@tigcc.ticalc.org> - 7:4.7.1-2
- Update printing patch: add ppdippstr.py (#723987), update debug.py, smburi.py

* Tue Sep 06 2011 Than Ngo <than@redhat.com> - 7:4.7.1-1
- 4.7.1

* Tue Jul 26 2011 Jaroslav Reznik <jreznik@redhat.com> - 7:4.7.0-1
- 4.7.0

* Fri Jul 08 2011 Jaroslav Reznik <jreznik@redhat.com> - 7:4.6.95-1
- 4.6.95 (rc2)

* Mon Jun 27 2011 Than Ngo <than@redhat.com> - 7:4.6.90-1
- 4.6.90 (rc1)

* Mon Jun 06 2011 Jaroslav Reznik <jreznik@redhat.com> 7:4.6.80-1
- 4.6.80 (beta1)

* Fri Apr 29 2011 Rex Dieter <rdieter@fedoraproject.org> 7:4.6.3-1
- 4.6.3

* Fri Apr 08 2011 Rex Dieter <rdieter@fedoraproject.org> 7:4.6.2-2
- No automagic escalation of privileges to add a printer (#652272)

* Wed Apr 06 2011 Than Ngo <than@redhat.com> - 7:4.6.2-1
- 4.6.2

* Mon Feb 28 2011 Rex Dieter <rdieter@fedoraproject.org> 7:4.6.1-1
- 4.6.1

* Mon Feb 07 2011 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 7:4.6.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_15_Mass_Rebuild

* Fri Jan 21 2011 Jaroslav Reznik <jreznik@redhat.com> 4.6.0-1
- 4.6.0

* Wed Jan 05 2011 Jaroslav Reznik <jreznik@redhat.com> 4.5.95-1
- 4.5.95 (4.6rc2)

* Wed Dec 22 2010 Rex Dieter <rdieter@fedoraproject.org> 4.5.90-1
- 4.5.90 (4.6rc1)

* Sat Dec 04 2010 Thomas Janssen <thomasj@fedoraproject.org> 4.5.85-1
- 4.5.85 (4.6beta2)

* Sun Nov 21 2010 Rex Dieter <rdieter@fedoraproject.org> -  4.5.80-1
- 4.5.80 (4.6beta1)

* Fri Oct 29 2010 Than Ngo <than@redhat.com> - 7:4.5.3-1
- 4.5.3

* Sat Oct 02 2010 Rex Dieter <rdieter@fedoraproject.org> - 7:4.5.2-1
- 4.5.2

* Fri Aug 27 2010 Jaroslav Reznik <jreznik@redhat.com> - 7:4.5.1-1
- 4.5.1

* Tue Aug 03 2010 Than Ngo <than@redhat.com> - 7:4.5.0-1
- 4.5.0

* Sat Jul 24 2010 Rex Dieter <rdieter@fedoraproject.org> - 7:4.4.95-1
- 4.5 RC3 (4.4.95)

* Wed Jul 07 2010 Rex Dieter <rdieter@fedoraproject.org> - 7:4.4.92-1
- 4.5 RC2 (4.4.92)

* Fri Jun 25 2010 Jaroslav Reznik <jreznik@redhat.com> - 7:4.4.90-1
- 4.5 RC1 (4.4.90)

* Mon Jun 07 2010 Jaroslav Reznik <jreznik@redhat.com> - 7:4.4.85-1
- 4.5 Beta 2 (4.4.85)

* Fri May 21 2010 Jaroslav Reznik <jreznik@redhat.com> - 7:4.4.80-1
- 4.5 Beta 1 (4.4.80)

* Fri Apr 30 2010 Jaroslav Reznik <jreznik@redhat.com> - 7:4.4.3-1
- 4.4.3

* Mon Mar 29 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.4.2-1
- 4.4.2

* Sat Feb 27 2010 Rex Dieter <rdieter@fedoraproject.org> - 7:4.4.1-1
- 4.4.1

* Fri Feb 12 2010 Rex Dieter <rdieter@fedoraproject.org> - 7:4.4.0-2
- Printer Settings in System settings in KDE doesn't work (kde#219751#c9)

* Fri Feb 05 2010 Than Ngo <than@redhat.com> - 7:4.4.0-1
- 4.4.0

* Sun Jan 31 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.98-1
- KDE 4.3.98 (4.4rc3)

* Wed Jan 20 2010 Lukas Tinkl <ltinkl@redhat.com> - 4.3.95-1
- KDE 4.3.95 (4.4rc2)

* Wed Jan 06 2010 Rex Dieter <rdieter@fedoraproject.org> - 4.3.90-1
- kde-4.3.90 (4.4rc1)

* Fri Dec 18 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.85-1
- kde-4.3.85 (4.4beta2)

* Mon Dec 14 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.3.80-3
- Repositioning the KDE Brand (#547361)
- omit kpackage
- drop old/unused kuser consolehelper files

* Fri Dec 04 2009 Than Ngo <than@redhat.com> - 4.3.80-2
- fix rhel conditionals

* Tue Dec  1 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.3.80-1
- KDE 4.4 beta1 (4.3.80)

* Sun Nov 22 2009 Ben Boeckel <MathStuf@gmail.com> - 4.3.75-0.1.svn1048496
- Update to 4.3.75 snapshot

* Sat Oct 31 2009 Rex Dieter <rdieter@fedoraproject.org> 4.3.3-1
- 4.3.3

* Sun Oct 04 2009 Than Ngo <than@redhat.com> - 4.3.2-1
- 4.3.2

* Fri Aug 28 2009 Than Ngo <than@redhat.com> - 4.3.1-1
- 4.3.1

* Thu Jul 30 2009 Than Ngo <than@redhat.com> - 4.3.0-1
- 4.3.0

* Tue Jul 28 2009 Than Ngo <than@redhat.com> - 4.2.98-5
- don't include kpackage fo rhel

* Mon Jul 27 2009 Than Ngo <than@redhat.com> - 4.2.98-4
- don't include system_config_printer_kde for rhel

* Mon Jul 27 2009 Than Ngo <than@redhat.com> - 4.2.98-3
- fix knetworkconf backend to recognize fedora network settings

* Fri Jul 24 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 7:4.2.98-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Tue Jul 21 2009 Than Ngo <than@redhat.com> - 4.2.98-1
- 4.3rc3

* Thu Jul 09 2009 Than Ngo <than@redhat.com> - 4.2.96-1
- 4.3rc2

* Thu Jun 25 2009 Than Ngo <than@redhat.com> - 4.2.95-1
- 4.3rc1

* Thu Jun 04 2009 Lorenzo Villani <lvillani@binaryhelix.net> - 7:4.2.90-1
- KDE 4.3 Beta 2

* Wed May 20 2009 Kevin Kofler <Kevin@tigcc.ticalc.org> - 4.2.85-2
- reenable sytem-config-printer-kde
- rebase printing patch, drop hunks fixed upstream
- BR python-devel instead of just python
- fix file list, system-config-printer-kde is now a System Settings module

* Wed May 13 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.85-1
- KDE 4.3 beta 1

* Tue Apr 21 2009 Than Ngo <than@redhat.com> - 4.2.2-4
- get rid of the dependency of system-config-printer
- drop the BR on PyKDE4, system-config-printer-libs
  it's just needed for runtime

* Mon Apr 20 2009 Than Ngo <than@redhat.com> - 4.2.2-3
- fix #496646, system-config-printer-kde doesn't start

* Wed Apr 01 2009 Rex Dieter <rdieter@fedoraproject.org> 4.2.2-2
- optimize scriptlets

* Mon Mar 30 2009 Lukáš Tinkl <ltinkl@redhat.com> - 4.2.2-1
- KDE 4.2.2

* Fri Feb 27 2009 Than Ngo <than@redhat.com> - 4.2.1-1
- 4.2.1

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 7:4.2.0-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Thu Jan 22 2009 Than Ngo <than@redhat.com> - 4.2.0-1
- 4.2.0

* Wed Jan 14 2009 Rex Dieter <rdieter@fedoraproject.org> - 4.1.96-2
- include system-config-printer-kde

* Wed Jan 07 2009 Than Ngo <than@redhat.com> - 4.1.96-1
- 4.2rc1

* Thu Dec 11 2008 Than Ngo <than@redhat.com> -  4.1.85-1
- 4.2beta2

* Thu Dec 04 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.80-4
- rebuild for fixed kde-filesystem (macros.kde4) (get rid of rpaths)

* Thu Dec 04 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.80-3
- drop Requires: kdebase-workspace

* Thu Nov 20 2008 Than Ngo <than@redhat.com> 4.1.80-2
- get rid of duplicated BRs

* Wed Nov 19 2008 Lorenzo Villani <lvillani@binaryhelix.net> - 7:4.1.80-1
- 4.1.80
- BR cmake 2.6
- BR python-devel
- make install/fast

* Tue Nov 11 2008 Than Ngo <than@redhat.com> 4.1.3-1
- 4.1.3

* Mon Oct 20 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-4
- make %%description for kcron, knetworkconf more clear (#467650)
- Requires: kdebase-workspace (for kcm modules)
- cleanup extraneous scriptlets

* Mon Sep 29 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-3
- make VERBOSE=1
- respin against new(er) kde-filesystem

* Sun Sep 28 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-2
- (re)add unpackaged HTML/en/kcontrol/ files

* Fri Sep 26 2008 Rex Dieter <rdieter@fedoraproject.org> 4.1.2-1
- 4.1.2

* Fri Aug 29 2008 Than Ngo <than@redhat.com> 4.1.1-1
- 4.1.1

* Thu Jul 24 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.1.0-2
- remove broken .pc file (segfaults RPM, #456100), drop Requires: pkgconfig

* Wed Jul 23 2008 Than Ngo <than@redhat.com> 4.1.0-1
- 4.1.0

* Fri Jul 18 2008 Rex Dieter <rdieter@fedoraproject.org> 7:4.0.99-1
- 4.0.99

* Fri Jul 11 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.98-1
- 4.0.98

* Sun Jul 06 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.85-1
- 4.0.85

* Fri Jun 27 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.84-1
- 4.0.84

* Thu Jun 19 2008 Than Ngo <than@redhat.com> 4.0.83-1
- 4.0.83 (beta2)

* Sun Jun 15 2008 Rex Dieter <rdieter@fedoraproject.org> 4.0.82-1
- 4.0.82

* Mon May 26 2008 Than Ngo <than@redhat.com> 4.0.80-1
- 4.1 beta1

* Wed May 07 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.72-1
- update to 4.0.72
- add files and description for ksystemlog
- kcron is now a KCM (update file list)
- remove secpolicy from file list and description (dropped upstream)

* Thu Apr 03 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-3
- rebuild (again) for the fixed %%{_kde4_buildtype}

* Mon Mar 31 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 4.0.3-2
- rebuild for NDEBUG and _kde4_libexecdir

* Fri Mar 28 2008 Than Ngo <than@redhat.com> 4.0.3-1
- 4.0.3

* Thu Feb 28 2008 Than Ngo <than@redhat.com> 4.0.2-1
- 4.0.2

* Thu Jan 31 2008 Rex Dieter <rdieter@fedoraproject.org> 7:4.0.1-1
- 4.0.1
- don't use consolehelper for kuser (for now anyway, didn't work anyway)
- -kpackage scriptlet fixes

* Tue Jan 08 2008 Kevin Kofler <Kevin@tigcc.ticalc.org> 7:4.0.0-1
- update to 4.0.0

* Thu Dec 20 2007 Kevin Kofler <Kevin@tigcc.ticalc.org> 7:3.97.0-3
- don't run kpackage through consolehelper, it can elevate privileges on demand
  (see also #344751, though that bug appears not to have affected KDE 4)

* Thu Dec 13 2007 Rex Dieter <rdieter[AT]fedoraproject.org> 7:3.97.0-2
- cosmetics (drop extraneous BR's, touchup %%description)

* Thu Dec 06 2007 Than Ngo <than@redhat.com> 7:3.97.0-1
- 3.97.0

* Fri Nov 30 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.2-1
- kde-3.96.2

* Fri Nov 23 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.1-1
- kde-3.96.1
- also use epoch in changelog (also backwards)

* Thu Nov 22 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-7
- use consolehelper for kuser and kpackage

* Wed Nov 21 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-6
- put kpackage in a subpkg (for the smart requirement)

* Mon Nov 19 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-5
- BR: kde-filesystem >= 4

* Mon Nov 19 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-4
- BR: libXcomposite-devel
- BR: libXdamage-devel
- BR: libxkbfile-devel
- BR: libXpm-devel
- BR: libXv-devel
- BR: libXxf86misc-devel
- BR: libXtst-devel
- BR: libXScrnSaver-devel

* Fri Nov 16 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-3
- +BR: kde4-macros(api)

* Thu Nov 15 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-2
- included AUTHORS
- added %%post and %%postun
- BR: kde-filesystem

* Thu Nov 15 2007 Sebastian Vahl <fedora@deadbabylon.de> 7:3.96.0-1
- Initial version for Fedora
