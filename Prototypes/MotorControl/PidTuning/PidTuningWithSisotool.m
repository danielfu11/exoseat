k = 1.45; %motor constant (k_e = k_t)
B = 0.0184; %friction coeff
J = 0.233; %moment of intertia (kg*m^2)

T_int = 0.194; %internal motor torque (Nm)
T_ext = 0; %external torque on motor (T_ext = 0 is ASSUMPTION)
T = T_int + T_ext; %total torque

R_m = 2.11; %ohms
R_b = 27; %current limiting resistors in series with BJT bases (ohms)
beta = 200; %gain of BJT driver circuit
R = R_m + 2*R_b/beta;

G_0 = 1/k;
omega_m = (k*k)/(J*R);

PlantG_s = tf([G_0 * omega_m], [1 omega_m])
SensorH_s = 333.7;
sisotool(PlantG_s, 1, SensorH_s);