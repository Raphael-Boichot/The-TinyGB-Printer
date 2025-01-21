clc
clear

disp('-----------------------------------------------------------')
disp('|      This code can be run with Matlab or Octave         |')
disp('-----------------------------------------------------------')

a=imread('Splash.png');

fid=fopen('splash.h','w');
[hauteur, largeur, pro]=size(a);
counter=0;

fprintf(fid,'const unsigned int splashscreen[] = {');
for i=1:1:hauteur
    for j=1:1:largeur
        counter=counter+1;
        fprintf(fid,'0x');
        if a(i,j)<=0xF; fprintf(fid,'0');end;
        fprintf(fid,'%X',a(i,j));
        fprintf(fid,',');
        if rem(counter,32)==0;fprintf(fid,'\n'); end;
    end
end
fseek(fid,-2,'cof');
fprintf(fid,'};');
fclose(fid);

disp('Image file converted to C file')
