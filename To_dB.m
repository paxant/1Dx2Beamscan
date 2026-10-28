function [value] = To_dB(value)

    value = value / max(value(:));
    value = abs(value);  
    value = 10*log10(value);
    value(value < -60) = -60;

end

