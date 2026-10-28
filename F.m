function Vnm = F(theta_range, phi_range, sh_th, sh_ph, A, Nx, Ny)
    Vnm = zeros(numel(phi_range), numel(theta_range));
    N = numel(A);
    for a = 1:N
        for b = 1:N
                st = sind(sh_th(a)); sp = sind(sh_ph(b));
                for i = 1:numel(theta_range)
                    for j = 1:numel(phi_range)
                        ux = sind(theta_range(i)) - st;
                        uy = sind(phi_range(j))   - sp;
                        s = 0;
                        for n = 0:Nx-1
                            for m = 0:Ny-1
                                pn = n-(Nx-1)/2;
                                pm = m-(Ny-1)/2;
                                s = s + exp(-1j*pi*(pn*ux + pm*uy));
                            end
                        end
                        Vnm(j,i) = Vnm(j,i) + (A(a)+A(b)) / 2 * s;
                    end
                end
            
        end
    end
end
