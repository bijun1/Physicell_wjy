/*
###############################################################################
# If you use PhysiCell in your project, please cite PhysiCell and the version #
# number, such as below:                                                      #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1].    #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# See VERSION.txt or call get_PhysiCell_version() to get the current version  #
#     x.y.z. Call display_citations() to get detailed information on all cite-#
#     able software used in your PhysiCell application.                       #
#                                                                             #
# Because PhysiCell extensively uses BioFVM, we suggest you also cite BioFVM  #
#     as below:                                                               #
#                                                                             #
# We implemented and solved the model using PhysiCell (Version x.y.z) [1],    #
# with BioFVM [2] to solve the transport equations.                           #
#                                                                             #
# [1] A Ghaffarizadeh, R Heiland, SH Friedman, SM Mumenthaler, and P Macklin, #
#     PhysiCell: an Open Source Physics-Based Cell Simulator for Multicellu-  #
#     lar Systems, PLoS Comput. Biol. 14(2): e1005991, 2018                   #
#     DOI: 10.1371/journal.pcbi.1005991                                       #
#                                                                             #
# [2] A Ghaffarizadeh, SH Friedman, and P Macklin, BioFVM: an efficient para- #
#     llelized diffusive transport solver for 3-D biological simulations,     #
#     Bioinformatics 32(8): 1256-8, 2016. DOI: 10.1093/bioinformatics/btv730  #
#                                                                             #
###############################################################################
#                                                                             #
# BSD 3-Clause License (see https://opensource.org/licenses/BSD-3-Clause)     #
#                                                                             #
# Copyright (c) 2015-2018, Paul Macklin and the PhysiCell Project             #
# All rights reserved.                                                        #
#                                                                             #
# Redistribution and use in source and binary forms, with or without          #
# modification, are permitted provided that the following conditions are met: #
#                                                                             #
# 1. Redistributions of source code must retain the above copyright notice,   #
# this list of conditions and the following disclaimer.                       #
#                                                                             #
# 2. Redistributions in binary form must reproduce the above copyright        #
# notice, this list of conditions and the following disclaimer in the         #
# documentation and/or other materials provided with the distribution.        #
#                                                                             #
# 3. Neither the name of the copyright holder nor the names of its            #
# contributors may be used to endorse or promote products derived from this   #
# software without specific prior written permission.                         #
#                                                                             #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" #
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   #
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  #
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE   #
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         #
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        #
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    #
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     #
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     #
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  #
# POSSIBILITY OF SUCH DAMAGE.                                                 #
#                                                                             #
###############################################################################
*/

#include "./custom.h"

// declare cell definitions here 

Cell_Definition motile_cell; 

void create_cell_types( void )
{
	// use the same random seed so that future experiments have the 
	// same initial histogram of oncoprotein, even if threading means 
	// that future division and other events are still not identical 
	// for all runs 
	
	SeedRandom( parameters.ints("random_seed") ); // or specify a seed here 
	
	// housekeeping 
	
	initialize_default_cell_definition();
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment ); 
	
	// Name the default cell type 
	
	cell_defaults.type = 0; 
	cell_defaults.name = "tumor cell"; 
	
	// set default cell cycle model 

	//cell_defaults.functions.cycle_model = flow_cytometry_separated_cycle_model; 
	cell_defaults.functions.cycle_model = Ki67_advanced; 
	
	// set default_cell_functions; 
	
	cell_defaults.functions.update_phenotype = wjy_update; 
	
	// needed for a 2-D simulation: 
	
	/* grab code from heterogeneity */ 
	
	cell_defaults.functions.set_orientation = up_orientation; 
	cell_defaults.phenotype.geometry.polarity = 1.0;
	cell_defaults.phenotype.motility.restrict_to_2D = true; 
	
	// make sure the defaults are self-consistent. 
	
	cell_defaults.phenotype.secretion.sync_to_microenvironment( &microenvironment );
	cell_defaults.phenotype.sync_to_functions( cell_defaults.functions ); 

	// set the rate terms in the default phenotype 

	// first find index for a few key variables. 
	int apoptosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Apoptosis" );
	int necrosis_model_index = cell_defaults.phenotype.death.find_death_model_index( "Necrosis" );
	int oxygen_substrate_index = microenvironment.find_density_index( "oxygen" ); 


	// initially no necrosis 
	cell_defaults.phenotype.death.rates[necrosis_model_index] = 0.0; 

	// set oxygen uptake / secretion parameters for the default cell type 
	cell_defaults.phenotype.secretion.uptake_rates[oxygen_substrate_index] = 10; 
	cell_defaults.phenotype.secretion.secretion_rates[oxygen_substrate_index] = 0; 
	cell_defaults.phenotype.secretion.saturation_densities[oxygen_substrate_index] = 38; 
	
	// add custom data here 
	cell_defaults.custom_data.add_variable("pi", "dimensionless", parameters.doubles("pi_ini"));
	cell_defaults.custom_data.add_variable("pe", "dimensionless", parameters.doubles("pe_ini"));
	cell_defaults.custom_data.add_variable("pf", "dimensionless", parameters.doubles("pf_ini"));

	cell_defaults.custom_data.add_variable("pi_ini", "dimensionless", parameters.doubles("pi_ini"));
	cell_defaults.custom_data.add_variable("pe_ini", "dimensionless", parameters.doubles("pe_ini"));

	cell_defaults.custom_data.add_variable("wjy_beta", "dimensionless", parameters.doubles("wjy_beta"));
	cell_defaults.custom_data.add_variable("wjy_alpha", "dimensionless", parameters.doubles("wjy_alpha"));
	cell_defaults.custom_data.add_variable("wjy_gamma", "dimensionless", parameters.doubles("wjy_gamma"));
	cell_defaults.custom_data.add_variable("wjy_rengp", "dimensionless", parameters.doubles("wjy_rengp"));
	cell_defaults.custom_data.add_variable("wjy_rengpp", "dimensionless", parameters.doubles("wjy_rengpp"));
	cell_defaults.custom_data.add_variable("wjy_rengpf", "dimensionless", parameters.doubles("wjy_rengpf"));
	cell_defaults.custom_data.add_variable("wjy_gmi", "dimensionless", parameters.doubles("wjy_gmi"));
	cell_defaults.custom_data.add_variable("wjy_gme", "dimensionless", parameters.doubles("wjy_gme"));
	cell_defaults.custom_data.add_variable("wjy_rea", "dimensionless", parameters.doubles("wjy_rea"));
	cell_defaults.custom_data.add_variable("wjy_ria", "dimensionless", parameters.doubles("wjy_ria"));
	cell_defaults.custom_data.add_variable("wjy_energy", "dimensionless", parameters.doubles("wjy_energy"));

	// Now, let's define another cell type. 
	// It's best to just copy the default and modify it. 
	
	// make this cell type randomly motile, less adhesive, greater survival, 
	// and less proliferative 
	
	motile_cell = cell_defaults; 
	motile_cell.type = 1; 
	motile_cell.name = "motile tumor cell"; 
	
	// make sure the new cell type has its own reference phenotype
	
	motile_cell.parameters.pReference_live_phenotype = &( motile_cell.phenotype ); 
	
	// enable random motility 
	motile_cell.phenotype.motility.is_motile = true; 
	motile_cell.phenotype.motility.persistence_time = parameters.doubles( "motile_cell_persistence_time" ); // 15.0; 
	motile_cell.phenotype.motility.migration_speed = parameters.doubles( "motile_cell_migration_speed" ); // 0.25 micron/minute 
	motile_cell.phenotype.motility.migration_bias = 0.0;// completely random 
	
	// Set cell-cell adhesion to 5% of other cells 
	motile_cell.phenotype.mechanics.cell_cell_adhesion_strength *= parameters.doubles( "motile_cell_relative_adhesion" ); // 0.05; 
	
	// Set apoptosis to zero 
	motile_cell.phenotype.death.rates[apoptosis_model_index] = parameters.doubles( "motile_cell_apoptosis_rate" ); // 0.0; 
	
	// Set proliferation to 10% of other cells. 
	// Alter the transition rate from G0G1 state to S state
	//motile_cell.phenotype.cycle.data.transition_rate(live_index,live_index) *= 
	//	parameters.doubles( "motile_cell_relative_cycle_entry_rate" ); // 0.1; 
	
	return; 
}

void setup_microenvironment( void )
{
	// set domain parameters 
	
/* now this is in XML 
	default_microenvironment_options.X_range = {-1000, 1000}; 
	default_microenvironment_options.Y_range = {-1000, 1000}; 
	default_microenvironment_options.simulate_2D = true; 
*/
	
	// make sure to override and go back to 2D 
	if( default_microenvironment_options.simulate_2D == false )
	{
		std::cout << "Warning: overriding XML config option and setting to 2D!" << std::endl; 
		default_microenvironment_options.simulate_2D = true; 
	}
	
/* now this is in XML 	
	// no gradients need for this example 

	default_microenvironment_options.calculate_gradients = false; 
	
	// set Dirichlet conditions 

	default_microenvironment_options.outer_Dirichlet_conditions = true;
	
	// if there are more substrates, resize accordingly 
	std::vector<double> bc_vector( 1 , 38.0 ); // 5% o2
	default_microenvironment_options.Dirichlet_condition_vector = bc_vector;
	
	// set initial conditions 
	default_microenvironment_options.initial_condition_vector = { 38.0 }; 
*/
	
	// put any custom code to set non-homogeneous initial conditions or 
	// extra Dirichlet nodes here. 
	
	// initialize BioFVM 
	
	initialize_microenvironment(); 	
	
	return; 
}

std::vector<int> randVector(int num) {
    std::vector<int> result;
    result.clear();
    result.reserve(num);
    srand((int)time(0));
    for (size_t i = 0; i < num; i++)
    {
        result.push_back(i);
    }
    int p1;
    int p2;
    int temp;
    int count = num;

    while (--num)
    {
        p1 = num;
        p2 = rand() % num;
        temp = result[p1];
        result[p1] = result[p2];
        result[p2] = temp;
    }
    return result;
}


class cord {
 public:
	int x;
	int y;
	cord(int i, int j) : x(i), y(j) {}
};

void change_for_ini_cell(Cell* pNew) {
	// change pi pe.
	int pi_index = pNew->custom_data.find_variable_index("pi");
	int pe_index = pNew->custom_data.find_variable_index("pe");
	int pf_index = pNew->custom_data.find_variable_index("pf");
	double pi = parameters.doubles("pi_ini") * (1 + 0 * NormalRandom(0, 1)); 
	double pe = parameters.doubles("pe_ini") * (1 + 0 * NormalRandom(0, 1)); 

	if (pi > 1) {
		pi = 1 - fabs(NormalRandom(0, 1) / 100);
	}
	if (pe > 1) {
		pe = 1 - fabs(NormalRandom(0, 1) / 100);
	}
	if (pi < 0) {
		pi = fabs(NormalRandom(0, 1) / 100);
	}
	if (pe < 0) {
		pe = fabs(NormalRandom(0, 1) / 100);
	}
	if (pi + pe > 1) {
		double pi_new = pi / (pi + pe + fabs(NormalRandom(0, 1) / 100));
		double pe_new = pe / (pi + pe + fabs(NormalRandom(0, 1) / 100));
		pi = pi_new;
		pe = pe_new;
	}

	pNew->custom_data[pi_index] = pi;
	pNew->custom_data[pe_index] = pe;
	pNew->custom_data[pf_index] = 1 - pNew->custom_data[pi_index] - pNew->custom_data[pe_index];
}

void setup_tissue( void )
{
	// create some cells near the origin
	
	Cell* pC;
        
	int sample_num = parameters.ints("sample_num");
	int radius = parameters.ints("radius");
        std::vector<cord> T;
	for (int i = -radius; i <= radius; i++) {
		for (int j = -radius; j <= radius; j++) {
			T.push_back(cord(i, j));
		}
	}
	std::vector<int> index = randVector((2 *radius + 1) * (2 * radius + 1));
	int count = 0;
	for (int id : index) {
		cord c = T[id];
		if (c.x * c.x + c.y * c.y > radius * radius) { continue; }
		pC = create_cell(); 
		change_for_ini_cell(pC);
		pC->assign_position( (double) c.x, (double) c.y, 0.0 );
		count++;
		if (count > sample_num) { break; }
	}
	return; 
}


std::vector<std::string> my_coloring_function( Cell* pCell )
{
	// start with flow cytometry coloring 
	
	std::vector<std::string> output = false_cell_coloring_cytometry(pCell); 

	switch(pCell->phenotype.cycle.current_phase().code) {
		// black
		case PhysiCell_constants::apoptotic:
		 	output[0] = "rgb(0,0,0)"; 
		 	output[2] = "rgb(0,0,0)"; 
			break;
		// BR
		case PhysiCell_constants::Ki67_positive_premitotic:
		 	output[0] = "rgb(100,0,255)"; 
		 	output[2] = "rgb(50,0,190)"; 
			break;
		// RB
		case PhysiCell_constants::Ki67_positive_postmitotic:
		 	output[0] = "rgb(255,0,100)"; 
		 	output[2] = "rgb(190,0,50)"; 
			break;
		// B
		case PhysiCell_constants::Ki67_negative:
		 	output[0] = "rgb(0,0,255)"; 
		 	output[2] = "rgb(0,0,190)"; 
			break;
		// Yellow
		default:
			output[0] = "rgb(255,255,0)";
			output[2] = "rgb(190,190,0)";
			break;
	}
	return output; 
}
