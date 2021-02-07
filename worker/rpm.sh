#/bin/sh

function help()
{
    echo "Sample version:
    1.00.1
    1: major version
    00:version
    1: Bug "
    echo "usage: ./rpm.sh 1.00.1 pms"
}

if [ $# -lt 2 ]
then
    help
    exit 1
fi

echo $1 | grep '^[0-9]\+\.[0-9]\+\.[0-9]\+$'
if [ $? != "0" ];then
    help
    exit 1
fi

mkdir -p ./source/$1
echo "copying source codes"

cp ./pms.service ./source/$1
cp -rf ./deps/./source/$1
cp -rf ./include ./source/$1
cp -rf ./src ./source/$1
cp -rf ./certs ./source/$1
cp -rf ./conf ./source/$1
cp -rf scripts ./source/$1
cp -f *.gyp ./source/$1
cp -f *.gypi ./source/$1
cp -f Makefile ./source/$1
cp -f compile_commands_template.json ./source/$1
cp -f install.sh ./source/$1

cd ./source
tar -zcvf ./$1.tar.gz $1 --exclude .svn --exclude .git --exclude *.o --exclude *.oclint --exclude *.log --exclude *.pdf --exclude *.doc

cd ..
mkdir -p ./rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

cp ./source/$1.tar.gz ./rpmbuild/SOURCES

name=pms
version=$1

rpmdir=$PWD/rpmbuild
echo "%_topdir  $rpmdir" > ~/.rpmmacros

echo "

Name:           $name
Version:        $version
Release:        1%{?dist}
Source:         $version.tar.gz

Summary:        $name
License:        GPL
URL:            https://pingos.io/
Packager:       db
Group:          Application/Server
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{realease}-root
 
%description
The GNU wget program downloads files from the Internet using the command-line.
 
%prep
%setup -q -n $version
cd $RPM_BUILD_DIR
 
%build
make && make master

%install
./install.sh

%clean

%files
/
%defattr(-,root,root,0755)

" > ./rpmbuild/SPECS/rpmbuild.spec

echo "rpm will be build"
rpmbuild -bb ./rpmbuild/SPECS/rpmbuild.spec || exit 1
