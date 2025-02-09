from pyradioconfig.parts.ocelot.calculators.calc_wisun import CALC_WiSUN_Ocelot

class CALC_WiSUN_Sol(CALC_WiSUN_Ocelot):

    def calc_wisun_phy_mode_id(self, model):
        # This function calculates the PhyModeID for Wi-SUN OFDM and Wi-SUN FSK PHYs
        # For OFDM, the variable contains an array of MCS0-6 values
        # For FSK, the variable contains an array of FSK off/on values

        profile_name = model.profile.name.lower()
        modulation_type = model.vars.modulation_type.value

        if profile_name in ["wisun_fan_1_0","wisun_han"]:
            #Call method from Ocelot
            super().calc_wisun_phy_mode_id(model)
        elif profile_name == "wisun_fan_1_1":
            if modulation_type == model.vars.modulation_type.var_enum.FSK2:
                # Call method from Ocelot
                super().calc_wisun_phy_mode_id(model)
            else:
                # This case is for Wi-SUN OFDM
                ofdm_option = model.vars.ofdm_option.value
                wisun_phy_mode_id = [0] * 7
                phy_type = int(ofdm_option) + 2
                wisun_phy_mode_id[0] = (phy_type << 4) + 0
                wisun_phy_mode_id[1] = (phy_type << 4) + 1
                wisun_phy_mode_id[2] = (phy_type << 4) + 2
                wisun_phy_mode_id[3] = (phy_type << 4) + 3
                wisun_phy_mode_id[4] = (phy_type << 4) + 4
                wisun_phy_mode_id[5] = (phy_type << 4) + 5
                wisun_phy_mode_id[6] = (phy_type << 4) + 6

                #Write the variable
                model.vars.wisun_phy_mode_id.value = wisun_phy_mode_id
