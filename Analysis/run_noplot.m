compile;

time=1;
%initial call
[V, gE] = conductance('dummy',{'gE'});

while time<3
    time=time+1
    [V, gE] =conductance(V,{'gE'});
end
