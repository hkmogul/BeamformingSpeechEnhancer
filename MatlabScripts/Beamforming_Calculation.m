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
fft_length = size(D_left,2);
% hermitian transpose of Dl and Dr
D_left_H = D_left';
D_right_H = D_right';
% vector D and D_H
D = zeros(2,1,24,fft_length);
D_H = zeros(1,2,24,fft_length);
for i = 1 : 24
    for j = 1 : fft_length
        D(:,:,i,j) = [D_left(i,j); D_right(i,j)];
        D_H(:,:,i,j) = [conj(D_left(i,j)), conj(D_right(i,j))];
    end
end


% gamma: head-related coherence function
% numerator
gamma_num = sum(D_left .* conj(D_right));
% denominator
den_l = sum(abs(D_left).^2);
den_r = sum(abs(D_right).^2); 
gamma_denominator = sqrt(den_l .* den_r);
% gamma
gamma = gamma_num ./ gamma_denominator;


% phi: cross-power-spectral density matrix Phi22(k)
phi = ones(2,2,fft_length);
phi_inverse = zeros(2,2,fft_length);
phi(1,2,:) = gamma;
phi(2,1,:) = gamma;

% inverse phi
for i = 1 : fft_length
    phi_inverse(:,:,i) = inv(phi(:,:,i));
end


% omega: weight vector
w_num = zeros(2,1,24, fft_length);
w_den = zeros(24, fft_length);
w = zeros(2,1,24, fft_length);
% numerator
for i = 1 : 24
    for j = 1 : fft_length
        w_num(:,:,i,j) = phi_inverse(:,:,j) * D(:,:,i,j);
    end
end
% denominator
for i = 1 : 24
    for j = 1 : fft_length
        w_den(i,j) = D_H(:,:,i,j) * phi_inverse(:,:,j) * D(:,:,i,j);
    end
end
% final w
for i = 1 : 24
    for j = 1 : fft_length
        w(:,:,i,j) = w_num(:,:,i,j) / w_den(i,j);
    end
end
% w_l and w_r
w_l(:,:) = w(1,1,:,:);
w_r(:,:) = w(2,1,:,:);

% ifft for each angle
left_ifft = zeros(24, fft_length);
right_ifft = zeros(24, fft_length);
for i = 1 : 24
    left_ifft(i,:) = ifft(w_l(i,:));
    right_ifft(i,:) = ifft(w_r(i,:));
end


% audiowrite!
Fs = 44100;
stereo = zeros(fft_length, 2, 24);
for i = 1 : 24
    stereo(:,:,i) = [left_ifft(i,:);right_ifft(i,:)]';
end
stereo = stereo ./ max(abs(stereo(:)));  %because of audiowrite clipping...
for i = 1 : 24
    audiowrite(['degree',num2str((i-1)*15),'.wav'],stereo(:,:,i),Fs);
end


% test plot
% figure;
% plot(left_ifft(8,:));
% figure;
% plot(right_ifft(8,:));
