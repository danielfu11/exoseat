% Setup script, Lab 3, Loop Gain & Closed-loop analysis
% 10/3/2011, RZ
% 10/6/2013, DS
% Instructions...
%   Run this file, then open sisotool and load the System Data
%   Load: C: Gc;    (this is your 'controller')
%         G: Gm;    (this is your 'plant')
%         H: Ksense (this is your 'sensor gain')
%   In SISO design window: view root locus (gives closed loop poles)
%       bode plots of loop gain and closed-loop response
%   From Analysis menu, select Response to Step Command
%   Can move closed-loop poles or loop gain to fine tune design

% motor parameters
k = 1.45;           %V*s/rad
RM = 2.11;             %Ohm, motor winding resistance
RB = 27;           %Ohm, current limiting resistors
beta = 200;         %BJT Current gain
J = 0.233;           %N*m*s/rad
f_enc_max = 1.9e3;  %Hz, Maximum encoder frequency (when 

% compensator parameters from Lab 2 (starting point)
R3 = 200e3;         %Ohms
C3 = 10e-9;         %Farads 

% speed sensor parameters
Rpm_max = 54.22;       %max rpm speed from speed sensor (aka encoder) output

%--------------------------------------------------------------------------
%   Parameters below here are automaticly calculated based on the above
%   values.
%
% motor model
R = RM + 2*RB/beta;
Go = 1/k;
wm = k^2/(J*R);
s = tf('s');
Gm = Go/(1+s/wm);

% Compensator
KI = 1/(R3*C3);
Gc = KI/s;                                                      

% Sensing gain       
Ksense = Rpm_max*16*131/(2*pi*f_enc_max);