# `get_estimates()` works with deterministic run

    Code
      print(dplyr::select(get_estimates(deterministic_results), -estimated, -expected,
      -uncertainty, -gradient, -likelihood, -log_like_cv, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 48 x 18
         module_name module_id module_type label            type      type_id
         <chr>           <int> <chr>       <chr>            <chr>       <int>
       1 Selectivity         1 <NA>        inflection_point parameter      NA
       2 Selectivity         1 <NA>        log_slope        parameter      NA
       3 Selectivity         2 <NA>        inflection_point parameter      NA
       4 Selectivity         2 <NA>        log_slope        parameter      NA
       5 Fleet               1 <NA>        log_Fmort        parameter      NA
       6 Fleet               1 <NA>        log_Fmort        parameter      NA
       7 Fleet               1 <NA>        log_Fmort        parameter      NA
       8 Fleet               1 <NA>        log_Fmort        parameter      NA
       9 Fleet               1 <NA>        log_Fmort        parameter      NA
      10 Fleet               1 <NA>        log_Fmort        parameter      NA
      11 Fleet               1 <NA>        log_Fmort        parameter      NA
      12 Fleet               1 <NA>        log_Fmort        parameter      NA
      13 Fleet               1 <NA>        log_Fmort        parameter      NA
      14 Fleet               1 <NA>        log_Fmort        parameter      NA
      15 Fleet               1 <NA>        log_Fmort        parameter      NA
      16 Fleet               1 <NA>        log_Fmort        parameter      NA
      17 Fleet               1 <NA>        log_Fmort        parameter      NA
      18 Fleet               1 <NA>        log_Fmort        parameter      NA
      19 Fleet               1 <NA>        log_Fmort        parameter      NA
      20 Fleet               1 <NA>        log_Fmort        parameter      NA
      21 Fleet               1 <NA>        log_Fmort        parameter      NA
      22 Fleet               1 <NA>        log_Fmort        parameter      NA
      23 Fleet               1 <NA>        log_Fmort        parameter      NA
      24 Fleet               1 <NA>        log_Fmort        parameter      NA
      25 Fleet               1 <NA>        log_Fmort        parameter      NA
      26 Fleet               1 <NA>        log_Fmort        parameter      NA
      27 Fleet               1 <NA>        log_Fmort        parameter      NA
      28 Fleet               1 <NA>        log_Fmort        parameter      NA
      29 Fleet               1 <NA>        log_Fmort        parameter      NA
      30 Fleet               1 <NA>        log_Fmort        parameter      NA
      31 Fleet               1 <NA>        log_Fmort        parameter      NA
      32 Fleet               1 <NA>        log_Fmort        parameter      NA
      33 Fleet               1 <NA>        log_Fmort        parameter      NA
      34 Fleet               1 <NA>        log_Fmort        parameter      NA
      35 Fleet               2 <NA>        log_q            parameter      NA
      36 Recruitment         1 <NA>        log_rzero        parameter      NA
      37 Population          1 <NA>        log_init_naa     parameter      NA
      38 Population          1 <NA>        log_init_naa     parameter      NA
      39 Population          1 <NA>        log_init_naa     parameter      NA
      40 Population          1 <NA>        log_init_naa     parameter      NA
      41 Population          1 <NA>        log_init_naa     parameter      NA
      42 Population          1 <NA>        log_init_naa     parameter      NA
      43 Population          1 <NA>        log_init_naa     parameter      NA
      44 Population          1 <NA>        log_init_naa     parameter      NA
      45 Population          1 <NA>        log_init_naa     parameter      NA
      46 Population          1 <NA>        log_init_naa     parameter      NA
      47 Population          1 <NA>        log_init_naa     parameter      NA
      48 Population          1 <NA>        log_init_naa     parameter      NA
         parameter_id fleet year_i age_i length_i   input observed estimation_type
                <int> <chr>  <int> <int>    <int>   <dbl>    <dbl> <chr>          
       1            0 <NA>      NA    NA       NA   2           NA fixed_effects  
       2            0 <NA>      NA    NA       NA   0           NA fixed_effects  
       3            0 <NA>      NA    NA       NA   1.5         NA fixed_effects  
       4            0 <NA>      NA    NA       NA   0.693       NA fixed_effects  
       5            0 <NA>      NA    NA       NA  -4.66        NA fixed_effects  
       6            1 <NA>      NA    NA       NA  -3.60        NA fixed_effects  
       7            2 <NA>      NA    NA       NA  -3.10        NA fixed_effects  
       8            3 <NA>      NA    NA       NA  -2.80        NA fixed_effects  
       9            4 <NA>      NA    NA       NA  -3.02        NA fixed_effects  
      10            5 <NA>      NA    NA       NA  -2.44        NA fixed_effects  
      11            6 <NA>      NA    NA       NA  -2.43        NA fixed_effects  
      12            7 <NA>      NA    NA       NA  -1.68        NA fixed_effects  
      13            8 <NA>      NA    NA       NA  -2.22        NA fixed_effects  
      14            9 <NA>      NA    NA       NA  -2.02        NA fixed_effects  
      15           10 <NA>      NA    NA       NA  -1.89        NA fixed_effects  
      16           11 <NA>      NA    NA       NA  -1.82        NA fixed_effects  
      17           12 <NA>      NA    NA       NA  -2.15        NA fixed_effects  
      18           13 <NA>      NA    NA       NA  -1.78        NA fixed_effects  
      19           14 <NA>      NA    NA       NA  -1.71        NA fixed_effects  
      20           15 <NA>      NA    NA       NA  -1.82        NA fixed_effects  
      21           16 <NA>      NA    NA       NA  -1.16        NA fixed_effects  
      22           17 <NA>      NA    NA       NA  -1.36        NA fixed_effects  
      23           18 <NA>      NA    NA       NA  -1.37        NA fixed_effects  
      24           19 <NA>      NA    NA       NA  -1.38        NA fixed_effects  
      25           20 <NA>      NA    NA       NA  -1.05        NA fixed_effects  
      26           21 <NA>      NA    NA       NA  -1.37        NA fixed_effects  
      27           22 <NA>      NA    NA       NA  -0.871       NA fixed_effects  
      28           23 <NA>      NA    NA       NA  -1.06        NA fixed_effects  
      29           24 <NA>      NA    NA       NA  -1.07        NA fixed_effects  
      30           25 <NA>      NA    NA       NA  -1.16        NA fixed_effects  
      31           26 <NA>      NA    NA       NA  -1.18        NA fixed_effects  
      32           27 <NA>      NA    NA       NA  -0.840       NA fixed_effects  
      33           28 <NA>      NA    NA       NA  -1.11        NA fixed_effects  
      34           29 <NA>      NA    NA       NA  -0.694       NA fixed_effects  
      35            0 <NA>      NA    NA       NA -14.9         NA fixed_effects  
      36            0 <NA>      NA    NA       NA  13.8         NA fixed_effects  
      37            0 <NA>      NA    NA       NA  13.8         NA fixed_effects  
      38            1 <NA>      NA    NA       NA  13.6         NA fixed_effects  
      39            2 <NA>      NA    NA       NA  13.4         NA fixed_effects  
      40            3 <NA>      NA    NA       NA  13.2         NA fixed_effects  
      41            4 <NA>      NA    NA       NA  13.0         NA fixed_effects  
      42            5 <NA>      NA    NA       NA  12.8         NA fixed_effects  
      43            6 <NA>      NA    NA       NA  12.6         NA fixed_effects  
      44            7 <NA>      NA    NA       NA  12.4         NA fixed_effects  
      45            8 <NA>      NA    NA       NA  12.1         NA fixed_effects  
      46            9 <NA>      NA    NA       NA  11.9         NA fixed_effects  
      47           10 <NA>      NA    NA       NA  11.7         NA fixed_effects  
      48           11 <NA>      NA    NA       NA  13.2         NA fixed_effects  
         distribution input_type  lpdf log_sd
         <chr>        <chr>      <dbl>  <dbl>
       1 <NA>         <NA>          NA     NA
       2 <NA>         <NA>          NA     NA
       3 <NA>         <NA>          NA     NA
       4 <NA>         <NA>          NA     NA
       5 <NA>         <NA>          NA     NA
       6 <NA>         <NA>          NA     NA
       7 <NA>         <NA>          NA     NA
       8 <NA>         <NA>          NA     NA
       9 <NA>         <NA>          NA     NA
      10 <NA>         <NA>          NA     NA
      11 <NA>         <NA>          NA     NA
      12 <NA>         <NA>          NA     NA
      13 <NA>         <NA>          NA     NA
      14 <NA>         <NA>          NA     NA
      15 <NA>         <NA>          NA     NA
      16 <NA>         <NA>          NA     NA
      17 <NA>         <NA>          NA     NA
      18 <NA>         <NA>          NA     NA
      19 <NA>         <NA>          NA     NA
      20 <NA>         <NA>          NA     NA
      21 <NA>         <NA>          NA     NA
      22 <NA>         <NA>          NA     NA
      23 <NA>         <NA>          NA     NA
      24 <NA>         <NA>          NA     NA
      25 <NA>         <NA>          NA     NA
      26 <NA>         <NA>          NA     NA
      27 <NA>         <NA>          NA     NA
      28 <NA>         <NA>          NA     NA
      29 <NA>         <NA>          NA     NA
      30 <NA>         <NA>          NA     NA
      31 <NA>         <NA>          NA     NA
      32 <NA>         <NA>          NA     NA
      33 <NA>         <NA>          NA     NA
      34 <NA>         <NA>          NA     NA
      35 <NA>         <NA>          NA     NA
      36 <NA>         <NA>          NA     NA
      37 <NA>         <NA>          NA     NA
      38 <NA>         <NA>          NA     NA
      39 <NA>         <NA>          NA     NA
      40 <NA>         <NA>          NA     NA
      41 <NA>         <NA>          NA     NA
      42 <NA>         <NA>          NA     NA
      43 <NA>         <NA>          NA     NA
      44 <NA>         <NA>          NA     NA
      45 <NA>         <NA>          NA     NA
      46 <NA>         <NA>          NA     NA
      47 <NA>         <NA>          NA     NA
      48 <NA>         <NA>          NA     NA

# `get_estimates()` works with estimation run

    Code
      print(dplyr::select(get_estimates(readRDS(fit_files[[1]])), -estimated,
      -expected, -uncertainty, -gradient, -likelihood, -log_like_cv, -gradient), n = 320,
      width = Inf)
    Output
      # A tibble: 13,170 x 18
          module_name module_id module_type label                type            
          <chr>           <int> <chr>       <chr>                <chr>           
        1 Selectivity         1 <NA>        inflection_point     parameter       
        2 Selectivity         1 <NA>        log_slope            parameter       
        3 Selectivity         2 <NA>        inflection_point     parameter       
        4 Selectivity         2 <NA>        log_slope            parameter       
        5 Fleet               1 <NA>        log_Fmort            parameter       
        6 Fleet               1 <NA>        log_Fmort            parameter       
        7 Fleet               1 <NA>        log_Fmort            parameter       
        8 Fleet               1 <NA>        log_Fmort            parameter       
        9 Fleet               1 <NA>        log_Fmort            parameter       
       10 Fleet               1 <NA>        log_Fmort            parameter       
       11 Fleet               1 <NA>        log_Fmort            parameter       
       12 Fleet               1 <NA>        log_Fmort            parameter       
       13 Fleet               1 <NA>        log_Fmort            parameter       
       14 Fleet               1 <NA>        log_Fmort            parameter       
       15 Fleet               1 <NA>        log_Fmort            parameter       
       16 Fleet               1 <NA>        log_Fmort            parameter       
       17 Fleet               1 <NA>        log_Fmort            parameter       
       18 Fleet               1 <NA>        log_Fmort            parameter       
       19 Fleet               1 <NA>        log_Fmort            parameter       
       20 Fleet               1 <NA>        log_Fmort            parameter       
       21 Fleet               1 <NA>        log_Fmort            parameter       
       22 Fleet               1 <NA>        log_Fmort            parameter       
       23 Fleet               1 <NA>        log_Fmort            parameter       
       24 Fleet               1 <NA>        log_Fmort            parameter       
       25 Fleet               1 <NA>        log_Fmort            parameter       
       26 Fleet               1 <NA>        log_Fmort            parameter       
       27 Fleet               1 <NA>        log_Fmort            parameter       
       28 Fleet               1 <NA>        log_Fmort            parameter       
       29 Fleet               1 <NA>        log_Fmort            parameter       
       30 Fleet               1 <NA>        log_Fmort            parameter       
       31 Fleet               1 <NA>        log_Fmort            parameter       
       32 Fleet               1 <NA>        log_Fmort            parameter       
       33 Fleet               1 <NA>        log_Fmort            parameter       
       34 Fleet               1 <NA>        log_Fmort            parameter       
       35 Fleet               2 <NA>        log_q                parameter       
       36 Recruitment         1 <NA>        log_rzero            parameter       
       37 Population          1 <NA>        log_init_naa         parameter       
       38 Population          1 <NA>        log_init_naa         parameter       
       39 Population          1 <NA>        log_init_naa         parameter       
       40 Population          1 <NA>        log_init_naa         parameter       
       41 Population          1 <NA>        log_init_naa         parameter       
       42 Population          1 <NA>        log_init_naa         parameter       
       43 Population          1 <NA>        log_init_naa         parameter       
       44 Population          1 <NA>        log_init_naa         parameter       
       45 Population          1 <NA>        log_init_naa         parameter       
       46 Population          1 <NA>        log_init_naa         parameter       
       47 Population          1 <NA>        log_init_naa         parameter       
       48 Population          1 <NA>        log_init_naa         parameter       
       49 Model              NA <NA>        biomass              derived_quantity
       50 Model              NA <NA>        biomass              derived_quantity
       51 Model              NA <NA>        biomass              derived_quantity
       52 Model              NA <NA>        biomass              derived_quantity
       53 Model              NA <NA>        biomass              derived_quantity
       54 Model              NA <NA>        biomass              derived_quantity
       55 Model              NA <NA>        biomass              derived_quantity
       56 Model              NA <NA>        biomass              derived_quantity
       57 Model              NA <NA>        biomass              derived_quantity
       58 Model              NA <NA>        biomass              derived_quantity
       59 Model              NA <NA>        biomass              derived_quantity
       60 Model              NA <NA>        biomass              derived_quantity
       61 Model              NA <NA>        biomass              derived_quantity
       62 Model              NA <NA>        biomass              derived_quantity
       63 Model              NA <NA>        biomass              derived_quantity
       64 Model              NA <NA>        biomass              derived_quantity
       65 Model              NA <NA>        biomass              derived_quantity
       66 Model              NA <NA>        biomass              derived_quantity
       67 Model              NA <NA>        biomass              derived_quantity
       68 Model              NA <NA>        biomass              derived_quantity
       69 Model              NA <NA>        biomass              derived_quantity
       70 Model              NA <NA>        biomass              derived_quantity
       71 Model              NA <NA>        biomass              derived_quantity
       72 Model              NA <NA>        biomass              derived_quantity
       73 Model              NA <NA>        biomass              derived_quantity
       74 Model              NA <NA>        biomass              derived_quantity
       75 Model              NA <NA>        biomass              derived_quantity
       76 Model              NA <NA>        biomass              derived_quantity
       77 Model              NA <NA>        biomass              derived_quantity
       78 Model              NA <NA>        biomass              derived_quantity
       79 Model              NA <NA>        biomass              derived_quantity
       80 Model              NA <NA>        expected_recruitment derived_quantity
       81 Model              NA <NA>        expected_recruitment derived_quantity
       82 Model              NA <NA>        expected_recruitment derived_quantity
       83 Model              NA <NA>        expected_recruitment derived_quantity
       84 Model              NA <NA>        expected_recruitment derived_quantity
       85 Model              NA <NA>        expected_recruitment derived_quantity
       86 Model              NA <NA>        expected_recruitment derived_quantity
       87 Model              NA <NA>        expected_recruitment derived_quantity
       88 Model              NA <NA>        expected_recruitment derived_quantity
       89 Model              NA <NA>        expected_recruitment derived_quantity
       90 Model              NA <NA>        expected_recruitment derived_quantity
       91 Model              NA <NA>        expected_recruitment derived_quantity
       92 Model              NA <NA>        expected_recruitment derived_quantity
       93 Model              NA <NA>        expected_recruitment derived_quantity
       94 Model              NA <NA>        expected_recruitment derived_quantity
       95 Model              NA <NA>        expected_recruitment derived_quantity
       96 Model              NA <NA>        expected_recruitment derived_quantity
       97 Model              NA <NA>        expected_recruitment derived_quantity
       98 Model              NA <NA>        expected_recruitment derived_quantity
       99 Model              NA <NA>        expected_recruitment derived_quantity
      100 Model              NA <NA>        expected_recruitment derived_quantity
      101 Model              NA <NA>        expected_recruitment derived_quantity
      102 Model              NA <NA>        expected_recruitment derived_quantity
      103 Model              NA <NA>        expected_recruitment derived_quantity
      104 Model              NA <NA>        expected_recruitment derived_quantity
      105 Model              NA <NA>        expected_recruitment derived_quantity
      106 Model              NA <NA>        expected_recruitment derived_quantity
      107 Model              NA <NA>        expected_recruitment derived_quantity
      108 Model              NA <NA>        expected_recruitment derived_quantity
      109 Model              NA <NA>        expected_recruitment derived_quantity
      110 Model              NA <NA>        expected_recruitment derived_quantity
      111 Model              NA <NA>        mortality_F          derived_quantity
      112 Model              NA <NA>        mortality_F          derived_quantity
      113 Model              NA <NA>        mortality_F          derived_quantity
      114 Model              NA <NA>        mortality_F          derived_quantity
      115 Model              NA <NA>        mortality_F          derived_quantity
      116 Model              NA <NA>        mortality_F          derived_quantity
      117 Model              NA <NA>        mortality_F          derived_quantity
      118 Model              NA <NA>        mortality_F          derived_quantity
      119 Model              NA <NA>        mortality_F          derived_quantity
      120 Model              NA <NA>        mortality_F          derived_quantity
      121 Model              NA <NA>        mortality_F          derived_quantity
      122 Model              NA <NA>        mortality_F          derived_quantity
      123 Model              NA <NA>        mortality_F          derived_quantity
      124 Model              NA <NA>        mortality_F          derived_quantity
      125 Model              NA <NA>        mortality_F          derived_quantity
      126 Model              NA <NA>        mortality_F          derived_quantity
      127 Model              NA <NA>        mortality_F          derived_quantity
      128 Model              NA <NA>        mortality_F          derived_quantity
      129 Model              NA <NA>        mortality_F          derived_quantity
      130 Model              NA <NA>        mortality_F          derived_quantity
      131 Model              NA <NA>        mortality_F          derived_quantity
      132 Model              NA <NA>        mortality_F          derived_quantity
      133 Model              NA <NA>        mortality_F          derived_quantity
      134 Model              NA <NA>        mortality_F          derived_quantity
      135 Model              NA <NA>        mortality_F          derived_quantity
      136 Model              NA <NA>        mortality_F          derived_quantity
      137 Model              NA <NA>        mortality_F          derived_quantity
      138 Model              NA <NA>        mortality_F          derived_quantity
      139 Model              NA <NA>        mortality_F          derived_quantity
      140 Model              NA <NA>        mortality_F          derived_quantity
      141 Model              NA <NA>        mortality_F          derived_quantity
      142 Model              NA <NA>        mortality_F          derived_quantity
      143 Model              NA <NA>        mortality_F          derived_quantity
      144 Model              NA <NA>        mortality_F          derived_quantity
      145 Model              NA <NA>        mortality_F          derived_quantity
      146 Model              NA <NA>        mortality_F          derived_quantity
      147 Model              NA <NA>        mortality_F          derived_quantity
      148 Model              NA <NA>        mortality_F          derived_quantity
      149 Model              NA <NA>        mortality_F          derived_quantity
      150 Model              NA <NA>        mortality_F          derived_quantity
      151 Model              NA <NA>        mortality_F          derived_quantity
      152 Model              NA <NA>        mortality_F          derived_quantity
      153 Model              NA <NA>        mortality_F          derived_quantity
      154 Model              NA <NA>        mortality_F          derived_quantity
      155 Model              NA <NA>        mortality_F          derived_quantity
      156 Model              NA <NA>        mortality_F          derived_quantity
      157 Model              NA <NA>        mortality_F          derived_quantity
      158 Model              NA <NA>        mortality_F          derived_quantity
      159 Model              NA <NA>        mortality_F          derived_quantity
      160 Model              NA <NA>        mortality_F          derived_quantity
      161 Model              NA <NA>        mortality_F          derived_quantity
      162 Model              NA <NA>        mortality_F          derived_quantity
      163 Model              NA <NA>        mortality_F          derived_quantity
      164 Model              NA <NA>        mortality_F          derived_quantity
      165 Model              NA <NA>        mortality_F          derived_quantity
      166 Model              NA <NA>        mortality_F          derived_quantity
      167 Model              NA <NA>        mortality_F          derived_quantity
      168 Model              NA <NA>        mortality_F          derived_quantity
      169 Model              NA <NA>        mortality_F          derived_quantity
      170 Model              NA <NA>        mortality_F          derived_quantity
      171 Model              NA <NA>        mortality_F          derived_quantity
      172 Model              NA <NA>        mortality_F          derived_quantity
      173 Model              NA <NA>        mortality_F          derived_quantity
      174 Model              NA <NA>        mortality_F          derived_quantity
      175 Model              NA <NA>        mortality_F          derived_quantity
      176 Model              NA <NA>        mortality_F          derived_quantity
      177 Model              NA <NA>        mortality_F          derived_quantity
      178 Model              NA <NA>        mortality_F          derived_quantity
      179 Model              NA <NA>        mortality_F          derived_quantity
      180 Model              NA <NA>        mortality_F          derived_quantity
      181 Model              NA <NA>        mortality_F          derived_quantity
      182 Model              NA <NA>        mortality_F          derived_quantity
      183 Model              NA <NA>        mortality_F          derived_quantity
      184 Model              NA <NA>        mortality_F          derived_quantity
      185 Model              NA <NA>        mortality_F          derived_quantity
      186 Model              NA <NA>        mortality_F          derived_quantity
      187 Model              NA <NA>        mortality_F          derived_quantity
      188 Model              NA <NA>        mortality_F          derived_quantity
      189 Model              NA <NA>        mortality_F          derived_quantity
      190 Model              NA <NA>        mortality_F          derived_quantity
      191 Model              NA <NA>        mortality_F          derived_quantity
      192 Model              NA <NA>        mortality_F          derived_quantity
      193 Model              NA <NA>        mortality_F          derived_quantity
      194 Model              NA <NA>        mortality_F          derived_quantity
      195 Model              NA <NA>        mortality_F          derived_quantity
      196 Model              NA <NA>        mortality_F          derived_quantity
      197 Model              NA <NA>        mortality_F          derived_quantity
      198 Model              NA <NA>        mortality_F          derived_quantity
      199 Model              NA <NA>        mortality_F          derived_quantity
      200 Model              NA <NA>        mortality_F          derived_quantity
      201 Model              NA <NA>        mortality_F          derived_quantity
      202 Model              NA <NA>        mortality_F          derived_quantity
      203 Model              NA <NA>        mortality_F          derived_quantity
      204 Model              NA <NA>        mortality_F          derived_quantity
      205 Model              NA <NA>        mortality_F          derived_quantity
      206 Model              NA <NA>        mortality_F          derived_quantity
      207 Model              NA <NA>        mortality_F          derived_quantity
      208 Model              NA <NA>        mortality_F          derived_quantity
      209 Model              NA <NA>        mortality_F          derived_quantity
      210 Model              NA <NA>        mortality_F          derived_quantity
      211 Model              NA <NA>        mortality_F          derived_quantity
      212 Model              NA <NA>        mortality_F          derived_quantity
      213 Model              NA <NA>        mortality_F          derived_quantity
      214 Model              NA <NA>        mortality_F          derived_quantity
      215 Model              NA <NA>        mortality_F          derived_quantity
      216 Model              NA <NA>        mortality_F          derived_quantity
      217 Model              NA <NA>        mortality_F          derived_quantity
      218 Model              NA <NA>        mortality_F          derived_quantity
      219 Model              NA <NA>        mortality_F          derived_quantity
      220 Model              NA <NA>        mortality_F          derived_quantity
      221 Model              NA <NA>        mortality_F          derived_quantity
      222 Model              NA <NA>        mortality_F          derived_quantity
      223 Model              NA <NA>        mortality_F          derived_quantity
      224 Model              NA <NA>        mortality_F          derived_quantity
      225 Model              NA <NA>        mortality_F          derived_quantity
      226 Model              NA <NA>        mortality_F          derived_quantity
      227 Model              NA <NA>        mortality_F          derived_quantity
      228 Model              NA <NA>        mortality_F          derived_quantity
      229 Model              NA <NA>        mortality_F          derived_quantity
      230 Model              NA <NA>        mortality_F          derived_quantity
      231 Model              NA <NA>        mortality_F          derived_quantity
      232 Model              NA <NA>        mortality_F          derived_quantity
      233 Model              NA <NA>        mortality_F          derived_quantity
      234 Model              NA <NA>        mortality_F          derived_quantity
      235 Model              NA <NA>        mortality_F          derived_quantity
      236 Model              NA <NA>        mortality_F          derived_quantity
      237 Model              NA <NA>        mortality_F          derived_quantity
      238 Model              NA <NA>        mortality_F          derived_quantity
      239 Model              NA <NA>        mortality_F          derived_quantity
      240 Model              NA <NA>        mortality_F          derived_quantity
      241 Model              NA <NA>        mortality_F          derived_quantity
      242 Model              NA <NA>        mortality_F          derived_quantity
      243 Model              NA <NA>        mortality_F          derived_quantity
      244 Model              NA <NA>        mortality_F          derived_quantity
      245 Model              NA <NA>        mortality_F          derived_quantity
      246 Model              NA <NA>        mortality_F          derived_quantity
      247 Model              NA <NA>        mortality_F          derived_quantity
      248 Model              NA <NA>        mortality_F          derived_quantity
      249 Model              NA <NA>        mortality_F          derived_quantity
      250 Model              NA <NA>        mortality_F          derived_quantity
      251 Model              NA <NA>        mortality_F          derived_quantity
      252 Model              NA <NA>        mortality_F          derived_quantity
      253 Model              NA <NA>        mortality_F          derived_quantity
      254 Model              NA <NA>        mortality_F          derived_quantity
      255 Model              NA <NA>        mortality_F          derived_quantity
      256 Model              NA <NA>        mortality_F          derived_quantity
      257 Model              NA <NA>        mortality_F          derived_quantity
      258 Model              NA <NA>        mortality_F          derived_quantity
      259 Model              NA <NA>        mortality_F          derived_quantity
      260 Model              NA <NA>        mortality_F          derived_quantity
      261 Model              NA <NA>        mortality_F          derived_quantity
      262 Model              NA <NA>        mortality_F          derived_quantity
      263 Model              NA <NA>        mortality_F          derived_quantity
      264 Model              NA <NA>        mortality_F          derived_quantity
      265 Model              NA <NA>        mortality_F          derived_quantity
      266 Model              NA <NA>        mortality_F          derived_quantity
      267 Model              NA <NA>        mortality_F          derived_quantity
      268 Model              NA <NA>        mortality_F          derived_quantity
      269 Model              NA <NA>        mortality_F          derived_quantity
      270 Model              NA <NA>        mortality_F          derived_quantity
      271 Model              NA <NA>        mortality_F          derived_quantity
      272 Model              NA <NA>        mortality_F          derived_quantity
      273 Model              NA <NA>        mortality_F          derived_quantity
      274 Model              NA <NA>        mortality_F          derived_quantity
      275 Model              NA <NA>        mortality_F          derived_quantity
      276 Model              NA <NA>        mortality_F          derived_quantity
      277 Model              NA <NA>        mortality_F          derived_quantity
      278 Model              NA <NA>        mortality_F          derived_quantity
      279 Model              NA <NA>        mortality_F          derived_quantity
      280 Model              NA <NA>        mortality_F          derived_quantity
      281 Model              NA <NA>        mortality_F          derived_quantity
      282 Model              NA <NA>        mortality_F          derived_quantity
      283 Model              NA <NA>        mortality_F          derived_quantity
      284 Model              NA <NA>        mortality_F          derived_quantity
      285 Model              NA <NA>        mortality_F          derived_quantity
      286 Model              NA <NA>        mortality_F          derived_quantity
      287 Model              NA <NA>        mortality_F          derived_quantity
      288 Model              NA <NA>        mortality_F          derived_quantity
      289 Model              NA <NA>        mortality_F          derived_quantity
      290 Model              NA <NA>        mortality_F          derived_quantity
      291 Model              NA <NA>        mortality_F          derived_quantity
      292 Model              NA <NA>        mortality_F          derived_quantity
      293 Model              NA <NA>        mortality_F          derived_quantity
      294 Model              NA <NA>        mortality_F          derived_quantity
      295 Model              NA <NA>        mortality_F          derived_quantity
      296 Model              NA <NA>        mortality_F          derived_quantity
      297 Model              NA <NA>        mortality_F          derived_quantity
      298 Model              NA <NA>        mortality_F          derived_quantity
      299 Model              NA <NA>        mortality_F          derived_quantity
      300 Model              NA <NA>        mortality_F          derived_quantity
      301 Model              NA <NA>        mortality_F          derived_quantity
      302 Model              NA <NA>        mortality_F          derived_quantity
      303 Model              NA <NA>        mortality_F          derived_quantity
      304 Model              NA <NA>        mortality_F          derived_quantity
      305 Model              NA <NA>        mortality_F          derived_quantity
      306 Model              NA <NA>        mortality_F          derived_quantity
      307 Model              NA <NA>        mortality_F          derived_quantity
      308 Model              NA <NA>        mortality_F          derived_quantity
      309 Model              NA <NA>        mortality_F          derived_quantity
      310 Model              NA <NA>        mortality_F          derived_quantity
      311 Model              NA <NA>        mortality_F          derived_quantity
      312 Model              NA <NA>        mortality_F          derived_quantity
      313 Model              NA <NA>        mortality_F          derived_quantity
      314 Model              NA <NA>        mortality_F          derived_quantity
      315 Model              NA <NA>        mortality_F          derived_quantity
      316 Model              NA <NA>        mortality_F          derived_quantity
      317 Model              NA <NA>        mortality_F          derived_quantity
      318 Model              NA <NA>        mortality_F          derived_quantity
      319 Model              NA <NA>        mortality_F          derived_quantity
      320 Model              NA <NA>        mortality_F          derived_quantity
          type_id parameter_id fleet year_i age_i length_i  input observed
            <int>        <int> <chr>  <int> <int>    <int>  <dbl>    <dbl>
        1      NA            0 <NA>      NA    NA       NA  2           NA
        2      NA            0 <NA>      NA    NA       NA  0           NA
        3      NA            0 <NA>      NA    NA       NA  1.5         NA
        4      NA            0 <NA>      NA    NA       NA  0.693       NA
        5      NA            0 <NA>      NA    NA       NA -4.66        NA
        6      NA            1 <NA>      NA    NA       NA -3.60        NA
        7      NA            2 <NA>      NA    NA       NA -3.10        NA
        8      NA            3 <NA>      NA    NA       NA -2.80        NA
        9      NA            4 <NA>      NA    NA       NA -3.02        NA
       10      NA            5 <NA>      NA    NA       NA -2.44        NA
       11      NA            6 <NA>      NA    NA       NA -2.43        NA
       12      NA            7 <NA>      NA    NA       NA -1.68        NA
       13      NA            8 <NA>      NA    NA       NA -2.22        NA
       14      NA            9 <NA>      NA    NA       NA -2.02        NA
       15      NA           10 <NA>      NA    NA       NA -1.89        NA
       16      NA           11 <NA>      NA    NA       NA -1.82        NA
       17      NA           12 <NA>      NA    NA       NA -2.15        NA
       18      NA           13 <NA>      NA    NA       NA -1.78        NA
       19      NA           14 <NA>      NA    NA       NA -1.71        NA
       20      NA           15 <NA>      NA    NA       NA -1.82        NA
       21      NA           16 <NA>      NA    NA       NA -1.16        NA
       22      NA           17 <NA>      NA    NA       NA -1.36        NA
       23      NA           18 <NA>      NA    NA       NA -1.37        NA
       24      NA           19 <NA>      NA    NA       NA -1.38        NA
       25      NA           20 <NA>      NA    NA       NA -1.05        NA
       26      NA           21 <NA>      NA    NA       NA -1.37        NA
       27      NA           22 <NA>      NA    NA       NA -0.871       NA
       28      NA           23 <NA>      NA    NA       NA -1.06        NA
       29      NA           24 <NA>      NA    NA       NA -1.07        NA
       30      NA           25 <NA>      NA    NA       NA -1.16        NA
       31      NA           26 <NA>      NA    NA       NA -1.18        NA
       32      NA           27 <NA>      NA    NA       NA -0.840       NA
       33      NA           28 <NA>      NA    NA       NA -1.11        NA
       34      NA           29 <NA>      NA    NA       NA -0.694       NA
       35      NA            0 <NA>      NA    NA       NA  0           NA
       36      NA            0 <NA>      NA    NA       NA 13.8         NA
       37      NA            0 <NA>      NA    NA       NA 13.8         NA
       38      NA            1 <NA>      NA    NA       NA 13.6         NA
       39      NA            2 <NA>      NA    NA       NA 13.4         NA
       40      NA            3 <NA>      NA    NA       NA 13.2         NA
       41      NA            4 <NA>      NA    NA       NA 13.0         NA
       42      NA            5 <NA>      NA    NA       NA 12.8         NA
       43      NA            6 <NA>      NA    NA       NA 12.6         NA
       44      NA            7 <NA>      NA    NA       NA 12.4         NA
       45      NA            8 <NA>      NA    NA       NA 12.1         NA
       46      NA            9 <NA>      NA    NA       NA 11.9         NA
       47      NA           10 <NA>      NA    NA       NA 11.7         NA
       48      NA           11 <NA>      NA    NA       NA 13.2         NA
       49      NA           NA <NA>      NA    NA       NA NA           NA
       50      NA           NA <NA>      NA    NA       NA NA           NA
       51      NA           NA <NA>      NA    NA       NA NA           NA
       52      NA           NA <NA>      NA    NA       NA NA           NA
       53      NA           NA <NA>      NA    NA       NA NA           NA
       54      NA           NA <NA>      NA    NA       NA NA           NA
       55      NA           NA <NA>      NA    NA       NA NA           NA
       56      NA           NA <NA>      NA    NA       NA NA           NA
       57      NA           NA <NA>      NA    NA       NA NA           NA
       58      NA           NA <NA>      NA    NA       NA NA           NA
       59      NA           NA <NA>      NA    NA       NA NA           NA
       60      NA           NA <NA>      NA    NA       NA NA           NA
       61      NA           NA <NA>      NA    NA       NA NA           NA
       62      NA           NA <NA>      NA    NA       NA NA           NA
       63      NA           NA <NA>      NA    NA       NA NA           NA
       64      NA           NA <NA>      NA    NA       NA NA           NA
       65      NA           NA <NA>      NA    NA       NA NA           NA
       66      NA           NA <NA>      NA    NA       NA NA           NA
       67      NA           NA <NA>      NA    NA       NA NA           NA
       68      NA           NA <NA>      NA    NA       NA NA           NA
       69      NA           NA <NA>      NA    NA       NA NA           NA
       70      NA           NA <NA>      NA    NA       NA NA           NA
       71      NA           NA <NA>      NA    NA       NA NA           NA
       72      NA           NA <NA>      NA    NA       NA NA           NA
       73      NA           NA <NA>      NA    NA       NA NA           NA
       74      NA           NA <NA>      NA    NA       NA NA           NA
       75      NA           NA <NA>      NA    NA       NA NA           NA
       76      NA           NA <NA>      NA    NA       NA NA           NA
       77      NA           NA <NA>      NA    NA       NA NA           NA
       78      NA           NA <NA>      NA    NA       NA NA           NA
       79      NA           NA <NA>      NA    NA       NA NA           NA
       80      NA           NA <NA>      NA    NA       NA NA           NA
       81      NA           NA <NA>      NA    NA       NA NA           NA
       82      NA           NA <NA>      NA    NA       NA NA           NA
       83      NA           NA <NA>      NA    NA       NA NA           NA
       84      NA           NA <NA>      NA    NA       NA NA           NA
       85      NA           NA <NA>      NA    NA       NA NA           NA
       86      NA           NA <NA>      NA    NA       NA NA           NA
       87      NA           NA <NA>      NA    NA       NA NA           NA
       88      NA           NA <NA>      NA    NA       NA NA           NA
       89      NA           NA <NA>      NA    NA       NA NA           NA
       90      NA           NA <NA>      NA    NA       NA NA           NA
       91      NA           NA <NA>      NA    NA       NA NA           NA
       92      NA           NA <NA>      NA    NA       NA NA           NA
       93      NA           NA <NA>      NA    NA       NA NA           NA
       94      NA           NA <NA>      NA    NA       NA NA           NA
       95      NA           NA <NA>      NA    NA       NA NA           NA
       96      NA           NA <NA>      NA    NA       NA NA           NA
       97      NA           NA <NA>      NA    NA       NA NA           NA
       98      NA           NA <NA>      NA    NA       NA NA           NA
       99      NA           NA <NA>      NA    NA       NA NA           NA
      100      NA           NA <NA>      NA    NA       NA NA           NA
      101      NA           NA <NA>      NA    NA       NA NA           NA
      102      NA           NA <NA>      NA    NA       NA NA           NA
      103      NA           NA <NA>      NA    NA       NA NA           NA
      104      NA           NA <NA>      NA    NA       NA NA           NA
      105      NA           NA <NA>      NA    NA       NA NA           NA
      106      NA           NA <NA>      NA    NA       NA NA           NA
      107      NA           NA <NA>      NA    NA       NA NA           NA
      108      NA           NA <NA>      NA    NA       NA NA           NA
      109      NA           NA <NA>      NA    NA       NA NA           NA
      110      NA           NA <NA>      NA    NA       NA NA           NA
      111      NA           NA <NA>      NA    NA       NA NA           NA
      112      NA           NA <NA>      NA    NA       NA NA           NA
      113      NA           NA <NA>      NA    NA       NA NA           NA
      114      NA           NA <NA>      NA    NA       NA NA           NA
      115      NA           NA <NA>      NA    NA       NA NA           NA
      116      NA           NA <NA>      NA    NA       NA NA           NA
      117      NA           NA <NA>      NA    NA       NA NA           NA
      118      NA           NA <NA>      NA    NA       NA NA           NA
      119      NA           NA <NA>      NA    NA       NA NA           NA
      120      NA           NA <NA>      NA    NA       NA NA           NA
      121      NA           NA <NA>      NA    NA       NA NA           NA
      122      NA           NA <NA>      NA    NA       NA NA           NA
      123      NA           NA <NA>      NA    NA       NA NA           NA
      124      NA           NA <NA>      NA    NA       NA NA           NA
      125      NA           NA <NA>      NA    NA       NA NA           NA
      126      NA           NA <NA>      NA    NA       NA NA           NA
      127      NA           NA <NA>      NA    NA       NA NA           NA
      128      NA           NA <NA>      NA    NA       NA NA           NA
      129      NA           NA <NA>      NA    NA       NA NA           NA
      130      NA           NA <NA>      NA    NA       NA NA           NA
      131      NA           NA <NA>      NA    NA       NA NA           NA
      132      NA           NA <NA>      NA    NA       NA NA           NA
      133      NA           NA <NA>      NA    NA       NA NA           NA
      134      NA           NA <NA>      NA    NA       NA NA           NA
      135      NA           NA <NA>      NA    NA       NA NA           NA
      136      NA           NA <NA>      NA    NA       NA NA           NA
      137      NA           NA <NA>      NA    NA       NA NA           NA
      138      NA           NA <NA>      NA    NA       NA NA           NA
      139      NA           NA <NA>      NA    NA       NA NA           NA
      140      NA           NA <NA>      NA    NA       NA NA           NA
      141      NA           NA <NA>      NA    NA       NA NA           NA
      142      NA           NA <NA>      NA    NA       NA NA           NA
      143      NA           NA <NA>      NA    NA       NA NA           NA
      144      NA           NA <NA>      NA    NA       NA NA           NA
      145      NA           NA <NA>      NA    NA       NA NA           NA
      146      NA           NA <NA>      NA    NA       NA NA           NA
      147      NA           NA <NA>      NA    NA       NA NA           NA
      148      NA           NA <NA>      NA    NA       NA NA           NA
      149      NA           NA <NA>      NA    NA       NA NA           NA
      150      NA           NA <NA>      NA    NA       NA NA           NA
      151      NA           NA <NA>      NA    NA       NA NA           NA
      152      NA           NA <NA>      NA    NA       NA NA           NA
      153      NA           NA <NA>      NA    NA       NA NA           NA
      154      NA           NA <NA>      NA    NA       NA NA           NA
      155      NA           NA <NA>      NA    NA       NA NA           NA
      156      NA           NA <NA>      NA    NA       NA NA           NA
      157      NA           NA <NA>      NA    NA       NA NA           NA
      158      NA           NA <NA>      NA    NA       NA NA           NA
      159      NA           NA <NA>      NA    NA       NA NA           NA
      160      NA           NA <NA>      NA    NA       NA NA           NA
      161      NA           NA <NA>      NA    NA       NA NA           NA
      162      NA           NA <NA>      NA    NA       NA NA           NA
      163      NA           NA <NA>      NA    NA       NA NA           NA
      164      NA           NA <NA>      NA    NA       NA NA           NA
      165      NA           NA <NA>      NA    NA       NA NA           NA
      166      NA           NA <NA>      NA    NA       NA NA           NA
      167      NA           NA <NA>      NA    NA       NA NA           NA
      168      NA           NA <NA>      NA    NA       NA NA           NA
      169      NA           NA <NA>      NA    NA       NA NA           NA
      170      NA           NA <NA>      NA    NA       NA NA           NA
      171      NA           NA <NA>      NA    NA       NA NA           NA
      172      NA           NA <NA>      NA    NA       NA NA           NA
      173      NA           NA <NA>      NA    NA       NA NA           NA
      174      NA           NA <NA>      NA    NA       NA NA           NA
      175      NA           NA <NA>      NA    NA       NA NA           NA
      176      NA           NA <NA>      NA    NA       NA NA           NA
      177      NA           NA <NA>      NA    NA       NA NA           NA
      178      NA           NA <NA>      NA    NA       NA NA           NA
      179      NA           NA <NA>      NA    NA       NA NA           NA
      180      NA           NA <NA>      NA    NA       NA NA           NA
      181      NA           NA <NA>      NA    NA       NA NA           NA
      182      NA           NA <NA>      NA    NA       NA NA           NA
      183      NA           NA <NA>      NA    NA       NA NA           NA
      184      NA           NA <NA>      NA    NA       NA NA           NA
      185      NA           NA <NA>      NA    NA       NA NA           NA
      186      NA           NA <NA>      NA    NA       NA NA           NA
      187      NA           NA <NA>      NA    NA       NA NA           NA
      188      NA           NA <NA>      NA    NA       NA NA           NA
      189      NA           NA <NA>      NA    NA       NA NA           NA
      190      NA           NA <NA>      NA    NA       NA NA           NA
      191      NA           NA <NA>      NA    NA       NA NA           NA
      192      NA           NA <NA>      NA    NA       NA NA           NA
      193      NA           NA <NA>      NA    NA       NA NA           NA
      194      NA           NA <NA>      NA    NA       NA NA           NA
      195      NA           NA <NA>      NA    NA       NA NA           NA
      196      NA           NA <NA>      NA    NA       NA NA           NA
      197      NA           NA <NA>      NA    NA       NA NA           NA
      198      NA           NA <NA>      NA    NA       NA NA           NA
      199      NA           NA <NA>      NA    NA       NA NA           NA
      200      NA           NA <NA>      NA    NA       NA NA           NA
      201      NA           NA <NA>      NA    NA       NA NA           NA
      202      NA           NA <NA>      NA    NA       NA NA           NA
      203      NA           NA <NA>      NA    NA       NA NA           NA
      204      NA           NA <NA>      NA    NA       NA NA           NA
      205      NA           NA <NA>      NA    NA       NA NA           NA
      206      NA           NA <NA>      NA    NA       NA NA           NA
      207      NA           NA <NA>      NA    NA       NA NA           NA
      208      NA           NA <NA>      NA    NA       NA NA           NA
      209      NA           NA <NA>      NA    NA       NA NA           NA
      210      NA           NA <NA>      NA    NA       NA NA           NA
      211      NA           NA <NA>      NA    NA       NA NA           NA
      212      NA           NA <NA>      NA    NA       NA NA           NA
      213      NA           NA <NA>      NA    NA       NA NA           NA
      214      NA           NA <NA>      NA    NA       NA NA           NA
      215      NA           NA <NA>      NA    NA       NA NA           NA
      216      NA           NA <NA>      NA    NA       NA NA           NA
      217      NA           NA <NA>      NA    NA       NA NA           NA
      218      NA           NA <NA>      NA    NA       NA NA           NA
      219      NA           NA <NA>      NA    NA       NA NA           NA
      220      NA           NA <NA>      NA    NA       NA NA           NA
      221      NA           NA <NA>      NA    NA       NA NA           NA
      222      NA           NA <NA>      NA    NA       NA NA           NA
      223      NA           NA <NA>      NA    NA       NA NA           NA
      224      NA           NA <NA>      NA    NA       NA NA           NA
      225      NA           NA <NA>      NA    NA       NA NA           NA
      226      NA           NA <NA>      NA    NA       NA NA           NA
      227      NA           NA <NA>      NA    NA       NA NA           NA
      228      NA           NA <NA>      NA    NA       NA NA           NA
      229      NA           NA <NA>      NA    NA       NA NA           NA
      230      NA           NA <NA>      NA    NA       NA NA           NA
      231      NA           NA <NA>      NA    NA       NA NA           NA
      232      NA           NA <NA>      NA    NA       NA NA           NA
      233      NA           NA <NA>      NA    NA       NA NA           NA
      234      NA           NA <NA>      NA    NA       NA NA           NA
      235      NA           NA <NA>      NA    NA       NA NA           NA
      236      NA           NA <NA>      NA    NA       NA NA           NA
      237      NA           NA <NA>      NA    NA       NA NA           NA
      238      NA           NA <NA>      NA    NA       NA NA           NA
      239      NA           NA <NA>      NA    NA       NA NA           NA
      240      NA           NA <NA>      NA    NA       NA NA           NA
      241      NA           NA <NA>      NA    NA       NA NA           NA
      242      NA           NA <NA>      NA    NA       NA NA           NA
      243      NA           NA <NA>      NA    NA       NA NA           NA
      244      NA           NA <NA>      NA    NA       NA NA           NA
      245      NA           NA <NA>      NA    NA       NA NA           NA
      246      NA           NA <NA>      NA    NA       NA NA           NA
      247      NA           NA <NA>      NA    NA       NA NA           NA
      248      NA           NA <NA>      NA    NA       NA NA           NA
      249      NA           NA <NA>      NA    NA       NA NA           NA
      250      NA           NA <NA>      NA    NA       NA NA           NA
      251      NA           NA <NA>      NA    NA       NA NA           NA
      252      NA           NA <NA>      NA    NA       NA NA           NA
      253      NA           NA <NA>      NA    NA       NA NA           NA
      254      NA           NA <NA>      NA    NA       NA NA           NA
      255      NA           NA <NA>      NA    NA       NA NA           NA
      256      NA           NA <NA>      NA    NA       NA NA           NA
      257      NA           NA <NA>      NA    NA       NA NA           NA
      258      NA           NA <NA>      NA    NA       NA NA           NA
      259      NA           NA <NA>      NA    NA       NA NA           NA
      260      NA           NA <NA>      NA    NA       NA NA           NA
      261      NA           NA <NA>      NA    NA       NA NA           NA
      262      NA           NA <NA>      NA    NA       NA NA           NA
      263      NA           NA <NA>      NA    NA       NA NA           NA
      264      NA           NA <NA>      NA    NA       NA NA           NA
      265      NA           NA <NA>      NA    NA       NA NA           NA
      266      NA           NA <NA>      NA    NA       NA NA           NA
      267      NA           NA <NA>      NA    NA       NA NA           NA
      268      NA           NA <NA>      NA    NA       NA NA           NA
      269      NA           NA <NA>      NA    NA       NA NA           NA
      270      NA           NA <NA>      NA    NA       NA NA           NA
      271      NA           NA <NA>      NA    NA       NA NA           NA
      272      NA           NA <NA>      NA    NA       NA NA           NA
      273      NA           NA <NA>      NA    NA       NA NA           NA
      274      NA           NA <NA>      NA    NA       NA NA           NA
      275      NA           NA <NA>      NA    NA       NA NA           NA
      276      NA           NA <NA>      NA    NA       NA NA           NA
      277      NA           NA <NA>      NA    NA       NA NA           NA
      278      NA           NA <NA>      NA    NA       NA NA           NA
      279      NA           NA <NA>      NA    NA       NA NA           NA
      280      NA           NA <NA>      NA    NA       NA NA           NA
      281      NA           NA <NA>      NA    NA       NA NA           NA
      282      NA           NA <NA>      NA    NA       NA NA           NA
      283      NA           NA <NA>      NA    NA       NA NA           NA
      284      NA           NA <NA>      NA    NA       NA NA           NA
      285      NA           NA <NA>      NA    NA       NA NA           NA
      286      NA           NA <NA>      NA    NA       NA NA           NA
      287      NA           NA <NA>      NA    NA       NA NA           NA
      288      NA           NA <NA>      NA    NA       NA NA           NA
      289      NA           NA <NA>      NA    NA       NA NA           NA
      290      NA           NA <NA>      NA    NA       NA NA           NA
      291      NA           NA <NA>      NA    NA       NA NA           NA
      292      NA           NA <NA>      NA    NA       NA NA           NA
      293      NA           NA <NA>      NA    NA       NA NA           NA
      294      NA           NA <NA>      NA    NA       NA NA           NA
      295      NA           NA <NA>      NA    NA       NA NA           NA
      296      NA           NA <NA>      NA    NA       NA NA           NA
      297      NA           NA <NA>      NA    NA       NA NA           NA
      298      NA           NA <NA>      NA    NA       NA NA           NA
      299      NA           NA <NA>      NA    NA       NA NA           NA
      300      NA           NA <NA>      NA    NA       NA NA           NA
      301      NA           NA <NA>      NA    NA       NA NA           NA
      302      NA           NA <NA>      NA    NA       NA NA           NA
      303      NA           NA <NA>      NA    NA       NA NA           NA
      304      NA           NA <NA>      NA    NA       NA NA           NA
      305      NA           NA <NA>      NA    NA       NA NA           NA
      306      NA           NA <NA>      NA    NA       NA NA           NA
      307      NA           NA <NA>      NA    NA       NA NA           NA
      308      NA           NA <NA>      NA    NA       NA NA           NA
      309      NA           NA <NA>      NA    NA       NA NA           NA
      310      NA           NA <NA>      NA    NA       NA NA           NA
      311      NA           NA <NA>      NA    NA       NA NA           NA
      312      NA           NA <NA>      NA    NA       NA NA           NA
      313      NA           NA <NA>      NA    NA       NA NA           NA
      314      NA           NA <NA>      NA    NA       NA NA           NA
      315      NA           NA <NA>      NA    NA       NA NA           NA
      316      NA           NA <NA>      NA    NA       NA NA           NA
      317      NA           NA <NA>      NA    NA       NA NA           NA
      318      NA           NA <NA>      NA    NA       NA NA           NA
      319      NA           NA <NA>      NA    NA       NA NA           NA
      320      NA           NA <NA>      NA    NA       NA NA           NA
          estimation_type distribution input_type  lpdf log_sd
          <chr>           <chr>        <chr>      <dbl>  <dbl>
        1 fixed_effects   <NA>         <NA>          NA     NA
        2 fixed_effects   <NA>         <NA>          NA     NA
        3 fixed_effects   <NA>         <NA>          NA     NA
        4 fixed_effects   <NA>         <NA>          NA     NA
        5 fixed_effects   <NA>         <NA>          NA     NA
        6 fixed_effects   <NA>         <NA>          NA     NA
        7 fixed_effects   <NA>         <NA>          NA     NA
        8 fixed_effects   <NA>         <NA>          NA     NA
        9 fixed_effects   <NA>         <NA>          NA     NA
       10 fixed_effects   <NA>         <NA>          NA     NA
       11 fixed_effects   <NA>         <NA>          NA     NA
       12 fixed_effects   <NA>         <NA>          NA     NA
       13 fixed_effects   <NA>         <NA>          NA     NA
       14 fixed_effects   <NA>         <NA>          NA     NA
       15 fixed_effects   <NA>         <NA>          NA     NA
       16 fixed_effects   <NA>         <NA>          NA     NA
       17 fixed_effects   <NA>         <NA>          NA     NA
       18 fixed_effects   <NA>         <NA>          NA     NA
       19 fixed_effects   <NA>         <NA>          NA     NA
       20 fixed_effects   <NA>         <NA>          NA     NA
       21 fixed_effects   <NA>         <NA>          NA     NA
       22 fixed_effects   <NA>         <NA>          NA     NA
       23 fixed_effects   <NA>         <NA>          NA     NA
       24 fixed_effects   <NA>         <NA>          NA     NA
       25 fixed_effects   <NA>         <NA>          NA     NA
       26 fixed_effects   <NA>         <NA>          NA     NA
       27 fixed_effects   <NA>         <NA>          NA     NA
       28 fixed_effects   <NA>         <NA>          NA     NA
       29 fixed_effects   <NA>         <NA>          NA     NA
       30 fixed_effects   <NA>         <NA>          NA     NA
       31 fixed_effects   <NA>         <NA>          NA     NA
       32 fixed_effects   <NA>         <NA>          NA     NA
       33 fixed_effects   <NA>         <NA>          NA     NA
       34 fixed_effects   <NA>         <NA>          NA     NA
       35 fixed_effects   <NA>         <NA>          NA     NA
       36 fixed_effects   <NA>         <NA>          NA     NA
       37 fixed_effects   <NA>         <NA>          NA     NA
       38 fixed_effects   <NA>         <NA>          NA     NA
       39 fixed_effects   <NA>         <NA>          NA     NA
       40 fixed_effects   <NA>         <NA>          NA     NA
       41 fixed_effects   <NA>         <NA>          NA     NA
       42 fixed_effects   <NA>         <NA>          NA     NA
       43 fixed_effects   <NA>         <NA>          NA     NA
       44 fixed_effects   <NA>         <NA>          NA     NA
       45 fixed_effects   <NA>         <NA>          NA     NA
       46 fixed_effects   <NA>         <NA>          NA     NA
       47 fixed_effects   <NA>         <NA>          NA     NA
       48 fixed_effects   <NA>         <NA>          NA     NA
       49 <NA>            <NA>         <NA>          NA     NA
       50 <NA>            <NA>         <NA>          NA     NA
       51 <NA>            <NA>         <NA>          NA     NA
       52 <NA>            <NA>         <NA>          NA     NA
       53 <NA>            <NA>         <NA>          NA     NA
       54 <NA>            <NA>         <NA>          NA     NA
       55 <NA>            <NA>         <NA>          NA     NA
       56 <NA>            <NA>         <NA>          NA     NA
       57 <NA>            <NA>         <NA>          NA     NA
       58 <NA>            <NA>         <NA>          NA     NA
       59 <NA>            <NA>         <NA>          NA     NA
       60 <NA>            <NA>         <NA>          NA     NA
       61 <NA>            <NA>         <NA>          NA     NA
       62 <NA>            <NA>         <NA>          NA     NA
       63 <NA>            <NA>         <NA>          NA     NA
       64 <NA>            <NA>         <NA>          NA     NA
       65 <NA>            <NA>         <NA>          NA     NA
       66 <NA>            <NA>         <NA>          NA     NA
       67 <NA>            <NA>         <NA>          NA     NA
       68 <NA>            <NA>         <NA>          NA     NA
       69 <NA>            <NA>         <NA>          NA     NA
       70 <NA>            <NA>         <NA>          NA     NA
       71 <NA>            <NA>         <NA>          NA     NA
       72 <NA>            <NA>         <NA>          NA     NA
       73 <NA>            <NA>         <NA>          NA     NA
       74 <NA>            <NA>         <NA>          NA     NA
       75 <NA>            <NA>         <NA>          NA     NA
       76 <NA>            <NA>         <NA>          NA     NA
       77 <NA>            <NA>         <NA>          NA     NA
       78 <NA>            <NA>         <NA>          NA     NA
       79 <NA>            <NA>         <NA>          NA     NA
       80 <NA>            <NA>         <NA>          NA     NA
       81 <NA>            <NA>         <NA>          NA     NA
       82 <NA>            <NA>         <NA>          NA     NA
       83 <NA>            <NA>         <NA>          NA     NA
       84 <NA>            <NA>         <NA>          NA     NA
       85 <NA>            <NA>         <NA>          NA     NA
       86 <NA>            <NA>         <NA>          NA     NA
       87 <NA>            <NA>         <NA>          NA     NA
       88 <NA>            <NA>         <NA>          NA     NA
       89 <NA>            <NA>         <NA>          NA     NA
       90 <NA>            <NA>         <NA>          NA     NA
       91 <NA>            <NA>         <NA>          NA     NA
       92 <NA>            <NA>         <NA>          NA     NA
       93 <NA>            <NA>         <NA>          NA     NA
       94 <NA>            <NA>         <NA>          NA     NA
       95 <NA>            <NA>         <NA>          NA     NA
       96 <NA>            <NA>         <NA>          NA     NA
       97 <NA>            <NA>         <NA>          NA     NA
       98 <NA>            <NA>         <NA>          NA     NA
       99 <NA>            <NA>         <NA>          NA     NA
      100 <NA>            <NA>         <NA>          NA     NA
      101 <NA>            <NA>         <NA>          NA     NA
      102 <NA>            <NA>         <NA>          NA     NA
      103 <NA>            <NA>         <NA>          NA     NA
      104 <NA>            <NA>         <NA>          NA     NA
      105 <NA>            <NA>         <NA>          NA     NA
      106 <NA>            <NA>         <NA>          NA     NA
      107 <NA>            <NA>         <NA>          NA     NA
      108 <NA>            <NA>         <NA>          NA     NA
      109 <NA>            <NA>         <NA>          NA     NA
      110 <NA>            <NA>         <NA>          NA     NA
      111 <NA>            <NA>         <NA>          NA     NA
      112 <NA>            <NA>         <NA>          NA     NA
      113 <NA>            <NA>         <NA>          NA     NA
      114 <NA>            <NA>         <NA>          NA     NA
      115 <NA>            <NA>         <NA>          NA     NA
      116 <NA>            <NA>         <NA>          NA     NA
      117 <NA>            <NA>         <NA>          NA     NA
      118 <NA>            <NA>         <NA>          NA     NA
      119 <NA>            <NA>         <NA>          NA     NA
      120 <NA>            <NA>         <NA>          NA     NA
      121 <NA>            <NA>         <NA>          NA     NA
      122 <NA>            <NA>         <NA>          NA     NA
      123 <NA>            <NA>         <NA>          NA     NA
      124 <NA>            <NA>         <NA>          NA     NA
      125 <NA>            <NA>         <NA>          NA     NA
      126 <NA>            <NA>         <NA>          NA     NA
      127 <NA>            <NA>         <NA>          NA     NA
      128 <NA>            <NA>         <NA>          NA     NA
      129 <NA>            <NA>         <NA>          NA     NA
      130 <NA>            <NA>         <NA>          NA     NA
      131 <NA>            <NA>         <NA>          NA     NA
      132 <NA>            <NA>         <NA>          NA     NA
      133 <NA>            <NA>         <NA>          NA     NA
      134 <NA>            <NA>         <NA>          NA     NA
      135 <NA>            <NA>         <NA>          NA     NA
      136 <NA>            <NA>         <NA>          NA     NA
      137 <NA>            <NA>         <NA>          NA     NA
      138 <NA>            <NA>         <NA>          NA     NA
      139 <NA>            <NA>         <NA>          NA     NA
      140 <NA>            <NA>         <NA>          NA     NA
      141 <NA>            <NA>         <NA>          NA     NA
      142 <NA>            <NA>         <NA>          NA     NA
      143 <NA>            <NA>         <NA>          NA     NA
      144 <NA>            <NA>         <NA>          NA     NA
      145 <NA>            <NA>         <NA>          NA     NA
      146 <NA>            <NA>         <NA>          NA     NA
      147 <NA>            <NA>         <NA>          NA     NA
      148 <NA>            <NA>         <NA>          NA     NA
      149 <NA>            <NA>         <NA>          NA     NA
      150 <NA>            <NA>         <NA>          NA     NA
      151 <NA>            <NA>         <NA>          NA     NA
      152 <NA>            <NA>         <NA>          NA     NA
      153 <NA>            <NA>         <NA>          NA     NA
      154 <NA>            <NA>         <NA>          NA     NA
      155 <NA>            <NA>         <NA>          NA     NA
      156 <NA>            <NA>         <NA>          NA     NA
      157 <NA>            <NA>         <NA>          NA     NA
      158 <NA>            <NA>         <NA>          NA     NA
      159 <NA>            <NA>         <NA>          NA     NA
      160 <NA>            <NA>         <NA>          NA     NA
      161 <NA>            <NA>         <NA>          NA     NA
      162 <NA>            <NA>         <NA>          NA     NA
      163 <NA>            <NA>         <NA>          NA     NA
      164 <NA>            <NA>         <NA>          NA     NA
      165 <NA>            <NA>         <NA>          NA     NA
      166 <NA>            <NA>         <NA>          NA     NA
      167 <NA>            <NA>         <NA>          NA     NA
      168 <NA>            <NA>         <NA>          NA     NA
      169 <NA>            <NA>         <NA>          NA     NA
      170 <NA>            <NA>         <NA>          NA     NA
      171 <NA>            <NA>         <NA>          NA     NA
      172 <NA>            <NA>         <NA>          NA     NA
      173 <NA>            <NA>         <NA>          NA     NA
      174 <NA>            <NA>         <NA>          NA     NA
      175 <NA>            <NA>         <NA>          NA     NA
      176 <NA>            <NA>         <NA>          NA     NA
      177 <NA>            <NA>         <NA>          NA     NA
      178 <NA>            <NA>         <NA>          NA     NA
      179 <NA>            <NA>         <NA>          NA     NA
      180 <NA>            <NA>         <NA>          NA     NA
      181 <NA>            <NA>         <NA>          NA     NA
      182 <NA>            <NA>         <NA>          NA     NA
      183 <NA>            <NA>         <NA>          NA     NA
      184 <NA>            <NA>         <NA>          NA     NA
      185 <NA>            <NA>         <NA>          NA     NA
      186 <NA>            <NA>         <NA>          NA     NA
      187 <NA>            <NA>         <NA>          NA     NA
      188 <NA>            <NA>         <NA>          NA     NA
      189 <NA>            <NA>         <NA>          NA     NA
      190 <NA>            <NA>         <NA>          NA     NA
      191 <NA>            <NA>         <NA>          NA     NA
      192 <NA>            <NA>         <NA>          NA     NA
      193 <NA>            <NA>         <NA>          NA     NA
      194 <NA>            <NA>         <NA>          NA     NA
      195 <NA>            <NA>         <NA>          NA     NA
      196 <NA>            <NA>         <NA>          NA     NA
      197 <NA>            <NA>         <NA>          NA     NA
      198 <NA>            <NA>         <NA>          NA     NA
      199 <NA>            <NA>         <NA>          NA     NA
      200 <NA>            <NA>         <NA>          NA     NA
      201 <NA>            <NA>         <NA>          NA     NA
      202 <NA>            <NA>         <NA>          NA     NA
      203 <NA>            <NA>         <NA>          NA     NA
      204 <NA>            <NA>         <NA>          NA     NA
      205 <NA>            <NA>         <NA>          NA     NA
      206 <NA>            <NA>         <NA>          NA     NA
      207 <NA>            <NA>         <NA>          NA     NA
      208 <NA>            <NA>         <NA>          NA     NA
      209 <NA>            <NA>         <NA>          NA     NA
      210 <NA>            <NA>         <NA>          NA     NA
      211 <NA>            <NA>         <NA>          NA     NA
      212 <NA>            <NA>         <NA>          NA     NA
      213 <NA>            <NA>         <NA>          NA     NA
      214 <NA>            <NA>         <NA>          NA     NA
      215 <NA>            <NA>         <NA>          NA     NA
      216 <NA>            <NA>         <NA>          NA     NA
      217 <NA>            <NA>         <NA>          NA     NA
      218 <NA>            <NA>         <NA>          NA     NA
      219 <NA>            <NA>         <NA>          NA     NA
      220 <NA>            <NA>         <NA>          NA     NA
      221 <NA>            <NA>         <NA>          NA     NA
      222 <NA>            <NA>         <NA>          NA     NA
      223 <NA>            <NA>         <NA>          NA     NA
      224 <NA>            <NA>         <NA>          NA     NA
      225 <NA>            <NA>         <NA>          NA     NA
      226 <NA>            <NA>         <NA>          NA     NA
      227 <NA>            <NA>         <NA>          NA     NA
      228 <NA>            <NA>         <NA>          NA     NA
      229 <NA>            <NA>         <NA>          NA     NA
      230 <NA>            <NA>         <NA>          NA     NA
      231 <NA>            <NA>         <NA>          NA     NA
      232 <NA>            <NA>         <NA>          NA     NA
      233 <NA>            <NA>         <NA>          NA     NA
      234 <NA>            <NA>         <NA>          NA     NA
      235 <NA>            <NA>         <NA>          NA     NA
      236 <NA>            <NA>         <NA>          NA     NA
      237 <NA>            <NA>         <NA>          NA     NA
      238 <NA>            <NA>         <NA>          NA     NA
      239 <NA>            <NA>         <NA>          NA     NA
      240 <NA>            <NA>         <NA>          NA     NA
      241 <NA>            <NA>         <NA>          NA     NA
      242 <NA>            <NA>         <NA>          NA     NA
      243 <NA>            <NA>         <NA>          NA     NA
      244 <NA>            <NA>         <NA>          NA     NA
      245 <NA>            <NA>         <NA>          NA     NA
      246 <NA>            <NA>         <NA>          NA     NA
      247 <NA>            <NA>         <NA>          NA     NA
      248 <NA>            <NA>         <NA>          NA     NA
      249 <NA>            <NA>         <NA>          NA     NA
      250 <NA>            <NA>         <NA>          NA     NA
      251 <NA>            <NA>         <NA>          NA     NA
      252 <NA>            <NA>         <NA>          NA     NA
      253 <NA>            <NA>         <NA>          NA     NA
      254 <NA>            <NA>         <NA>          NA     NA
      255 <NA>            <NA>         <NA>          NA     NA
      256 <NA>            <NA>         <NA>          NA     NA
      257 <NA>            <NA>         <NA>          NA     NA
      258 <NA>            <NA>         <NA>          NA     NA
      259 <NA>            <NA>         <NA>          NA     NA
      260 <NA>            <NA>         <NA>          NA     NA
      261 <NA>            <NA>         <NA>          NA     NA
      262 <NA>            <NA>         <NA>          NA     NA
      263 <NA>            <NA>         <NA>          NA     NA
      264 <NA>            <NA>         <NA>          NA     NA
      265 <NA>            <NA>         <NA>          NA     NA
      266 <NA>            <NA>         <NA>          NA     NA
      267 <NA>            <NA>         <NA>          NA     NA
      268 <NA>            <NA>         <NA>          NA     NA
      269 <NA>            <NA>         <NA>          NA     NA
      270 <NA>            <NA>         <NA>          NA     NA
      271 <NA>            <NA>         <NA>          NA     NA
      272 <NA>            <NA>         <NA>          NA     NA
      273 <NA>            <NA>         <NA>          NA     NA
      274 <NA>            <NA>         <NA>          NA     NA
      275 <NA>            <NA>         <NA>          NA     NA
      276 <NA>            <NA>         <NA>          NA     NA
      277 <NA>            <NA>         <NA>          NA     NA
      278 <NA>            <NA>         <NA>          NA     NA
      279 <NA>            <NA>         <NA>          NA     NA
      280 <NA>            <NA>         <NA>          NA     NA
      281 <NA>            <NA>         <NA>          NA     NA
      282 <NA>            <NA>         <NA>          NA     NA
      283 <NA>            <NA>         <NA>          NA     NA
      284 <NA>            <NA>         <NA>          NA     NA
      285 <NA>            <NA>         <NA>          NA     NA
      286 <NA>            <NA>         <NA>          NA     NA
      287 <NA>            <NA>         <NA>          NA     NA
      288 <NA>            <NA>         <NA>          NA     NA
      289 <NA>            <NA>         <NA>          NA     NA
      290 <NA>            <NA>         <NA>          NA     NA
      291 <NA>            <NA>         <NA>          NA     NA
      292 <NA>            <NA>         <NA>          NA     NA
      293 <NA>            <NA>         <NA>          NA     NA
      294 <NA>            <NA>         <NA>          NA     NA
      295 <NA>            <NA>         <NA>          NA     NA
      296 <NA>            <NA>         <NA>          NA     NA
      297 <NA>            <NA>         <NA>          NA     NA
      298 <NA>            <NA>         <NA>          NA     NA
      299 <NA>            <NA>         <NA>          NA     NA
      300 <NA>            <NA>         <NA>          NA     NA
      301 <NA>            <NA>         <NA>          NA     NA
      302 <NA>            <NA>         <NA>          NA     NA
      303 <NA>            <NA>         <NA>          NA     NA
      304 <NA>            <NA>         <NA>          NA     NA
      305 <NA>            <NA>         <NA>          NA     NA
      306 <NA>            <NA>         <NA>          NA     NA
      307 <NA>            <NA>         <NA>          NA     NA
      308 <NA>            <NA>         <NA>          NA     NA
      309 <NA>            <NA>         <NA>          NA     NA
      310 <NA>            <NA>         <NA>          NA     NA
      311 <NA>            <NA>         <NA>          NA     NA
      312 <NA>            <NA>         <NA>          NA     NA
      313 <NA>            <NA>         <NA>          NA     NA
      314 <NA>            <NA>         <NA>          NA     NA
      315 <NA>            <NA>         <NA>          NA     NA
      316 <NA>            <NA>         <NA>          NA     NA
      317 <NA>            <NA>         <NA>          NA     NA
      318 <NA>            <NA>         <NA>          NA     NA
      319 <NA>            <NA>         <NA>          NA     NA
      320 <NA>            <NA>         <NA>          NA     NA
      # i 12,850 more rows

