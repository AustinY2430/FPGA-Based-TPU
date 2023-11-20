% Read the matrices from the input files
A = readmatrix('ayy.txt');
B = readmatrix('bee.txt');

N = 4096;
% Read data from the file
% Convert column into matrix
Cdata = dlmread('Output.txt');
Ccol = Cdata(:);
C = zeros(N);
counter = 1;
for j = 1:(N/8)
    for i = 1:(N/8)
        for l = 1:8
            for k = 1:8
                C((i-1)*8 + k, (j-1)*8 + l) = Ccol(counter);
                counter = counter + 1;
            end
        end
    end
end

% Calculate the product of the matrices
M = A * B;

% Compare the matrices
if isequal(M, C)
    disp('The matrices are equal.');
else
    disp('The matrices are not equal.');
end

% Write the result to an output file
dlmwrite('matlab_output.txt', M, 'delimiter', ',', 'precision', '%.6f');

% find indices of differing elements
%[row, col] = find(M ~= C);
% display differences along with indices
%for i = 1:length(row)
%    fprintf('Difference at position (%d,%d): %d\n', row(i), col(i), M(row(i),col(i)) - C(row(i),col(i)));
%end
