###################################################################
# Tutorial: Test "compute" functionality - longer runs
###################################################################

import os
import sys
import math

# Fisrt, we add the location of the library to test to the PYTHON path
cwd = os.getcwd()
print "Current working directory", cwd
sys.path.insert(1,cwd+"/../../_build/src/mmath")
sys.path.insert(1,cwd+"/../../_build/src/chemobjects")
sys.path.insert(1,cwd+"/../../_build/src/hamiltonian")
sys.path.insert(1,cwd+"/../../_build/src/dyn")
#sys.path.insert(1,cwd+"/../../_build/src/hamiltonian/hamiltonian_atomistic")


print "\nTest 1: Importing the library and its content"
from cygmmath import *
from cygchemobjects import *
from cyghamiltonian import *
from cygdyn import *
#from cyghamiltonian_atomistic import *


from LoadPT import * # Load_PT
from LoadMolecule import * # Load_Molecule
from LoadUFF import*



##############################################################

# Create Universe and populate it
U = Universe()
verbose = 0
Load_PT(U, "elements.dat", verbose)


# Create force field
uff = ForceField({"bond_functional":"Harmonic" ,"angle_functional":"Fourier",
                   "vdw_functional":"LJ","dihedral_functional":"General0",
                    "R_vdw_on":6.0,"R_vdw_off":7.0}
                )

Load_UFF(uff)
verb = 0
assign_rings = 1



#======= System ==============
#for i in range(1,13):
for i in [1]:
    print "=================== System ",i,"======================="

    syst = System()
    Load_Molecule(U, syst, os.getcwd()+"/Molecules/test"+str(i)+"a.pdb", "pdb_1")
    syst.determine_functional_groups(1)  # 
    syst.show_atoms()
    syst.show_fragments()
    syst.show_molecules()

    #syst.print_xyz("molecule1.xyz",1)

    print "Number of atoms in the system = ", syst.Number_of_atoms
    atlst1 = range(1,syst.Number_of_atoms+1)


    # Creating Hamiltonian
    ham = Hamiltonian_Atomistic(1, 3*syst.Number_of_atoms)
    ham.set_Hamiltonian_type("MM")
    ham.set_interactions_for_atoms(syst, atlst1, atlst1, uff, verb, assign_rings)  
    ham.show_interactions_statistics()
    ham.set_system(syst)
    ham.compute()

#    sys.exit(0)
    print "Energy = ", ham.H(0,0), " a.u."
    print "Force 1 = ", ham.dHdq(0,0,0), " a.u."
    print "Force 3 = ", ham.dHdq(0,0,3), " a.u."


#    sys.exit(0)

    #--------------------- Molecular dynamics ----------------------
    el = Electronic(1,0)

    # Nuclear DOFs
    mol = Nuclear(3*syst.Number_of_atoms)
    syst.extract_atomic_q(mol.q)
    syst.extract_atomic_p(mol.p)
    syst.extract_atomic_f(mol.f)
    syst.extract_atomic_mass(mol.mass)

    print "q= ", mol.q[0],
    print "p= ", mol.p[0],
    print "f= ", mol.f[0],
    print "mass= ", mol.mass[0],
    

    f = open("_en_traj.txt","w")
    dt = 1.0 #  0.5 fs
    for i in xrange(100):
        syst.set_atomic_q(mol.q)
        syst.print_xyz("_mol_traj.xyz",i)

        for j in xrange(10):


            mol.propagate_p(0.5*dt)
            mol.propagate_q(dt)            

            epot = compute_potential_energy(mol, el, ham, 1)  # 1 - FSSH forces
            compute_forces(mol, el, ham, 1)

            mol.propagate_p(0.5*dt)


            ekin = compute_kinetic_energy(mol)

        f.write("i= %3i ekin= %8.5f  epot= %8.5f  etot= %8.5f\n" % (i, ekin, epot, ekin+epot))

        


      
    f.close()



