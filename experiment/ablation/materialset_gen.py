def readTable(table_path):
    material_table = []
    with open(table_path, 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break
            material_table.append(str(line.split()[0]))
    return material_table


if __name__ == '__main__':

    from random import uniform, randint
    import os.path
    material_table = readTable(
        '/home/lzr/Projects/layeredbsdf/pyscript/material_names_table.txt')
    print('[eta-k] Read ' + str(len(material_table)) + ' material presets')
    with open(os.path.join(os.path.dirname(__file__) , f'material_set.txt'), 'a') as outfile:
            
        for i in range(90):
            sigma_t = uniform(0,5)
            albedo = [
                uniform(0, 1), uniform(0, 1), uniform(0, 1)
            ]
            g = 0

            # roughness 0.001-1
            alpha_0 = 10**uniform(-2, -0.5)
            alpha_1 = 10**uniform(-2, 0)

            # normal semi-sphere
            theta_0 = 0
            phi_0 = 0
            theta_1 = 0
            phi_1 = 0

            # ior 1.05-2
            eta_0 = uniform(1.05, 2)
            
            # get a random pair of eta-k from the table!

            ranidx = randint(0, len(material_table) - 1)
            material_1 = material_table[ranidx]

            filename = f'{alpha_0:.4f}_{theta_0}_{phi_0}_{eta_0:.4f}_{sigma_t:.4f}_{albedo[0]:.4f}_{albedo[1]:.4f}_{albedo[2]:.4f}_{g}_{alpha_1:.4f}_{theta_1}_{phi_1}_{material_1}'
                        
            outfile.write(filename+'\n')
