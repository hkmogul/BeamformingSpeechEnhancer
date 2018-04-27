close all;
clear all;

load IRC_1002_R_HRIR.mat

% impulse response: elevation = 0, 24 different azimuth
left_impulse = l_hrir_S.content_m(73:96,:);
right_impulse = r_hrir_S.content_m(73:96,:);
% trim off empty space at beginning of impulse to reduce latency
left_impulse(:,1:240) = [];
right_impulse(:,1:240) = [];

% Dl, Dr: fft of impulse response
D_left = fft(left_impulse.').';
D_right = fft(right_impulse.').';

% magnitude of fft impulse response
D_lm = abs(D_left);
D_rm = abs(D_right);

% calculate M
M = (D_lm.^2 + D_rm.^2)./(D_lm + D_rm).^2;

% ifft of M
M_ifft = ifft(M.').';

% audiowrite!
Fs = 44100;
for i = 1 : 24
    audiowrite(['M_degree',num2str((i-1)*15),'.wav'],M_ifft(i,:)',Fs);
end