

A = csvread('Position.txt');
Exact = csvread('Trajectory.txt');

figure(1)
%hold on
plot3(A(:,1),A(:,2),A(:,3))
xlabel('x')
ylabel('y')
zlabel('z')


figure(2)
plot(A(:,1),A(:,3))
