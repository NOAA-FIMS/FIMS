# `get_estimates()` works with deterministic run

    Code
      print(dplyr::select(get_estimates(deterministic_results), -estimated, -expected,
      -uncertainty, -gradient, -likelihood, -log_like_cv, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 14,247 x 17
          module_name module_id module_type label                    type   type_id
          <chr>           <int> <chr>       <chr>                    <chr>    <int>
        1 Fleet               1 <NA>        log_Fmort                vector       4
        2 Fleet               1 <NA>        log_Fmort                vector       4
        3 Fleet               1 <NA>        log_Fmort                vector       4
        4 Fleet               1 <NA>        log_Fmort                vector       4
        5 Fleet               1 <NA>        log_Fmort                vector       4
        6 Fleet               1 <NA>        log_Fmort                vector       4
        7 Fleet               1 <NA>        log_Fmort                vector       4
        8 Fleet               1 <NA>        log_Fmort                vector       4
        9 Fleet               1 <NA>        log_Fmort                vector       4
       10 Fleet               1 <NA>        log_Fmort                vector       4
       11 Fleet               1 <NA>        log_Fmort                vector       4
       12 Fleet               1 <NA>        log_Fmort                vector       4
       13 Fleet               1 <NA>        log_Fmort                vector       4
       14 Fleet               1 <NA>        log_Fmort                vector       4
       15 Fleet               1 <NA>        log_Fmort                vector       4
       16 Fleet               1 <NA>        log_Fmort                vector       4
       17 Fleet               1 <NA>        log_Fmort                vector       4
       18 Fleet               1 <NA>        log_Fmort                vector       4
       19 Fleet               1 <NA>        log_Fmort                vector       4
       20 Fleet               1 <NA>        log_Fmort                vector       4
       21 Fleet               1 <NA>        log_Fmort                vector       4
       22 Fleet               1 <NA>        log_Fmort                vector       4
       23 Fleet               1 <NA>        log_Fmort                vector       4
       24 Fleet               1 <NA>        log_Fmort                vector       4
       25 Fleet               1 <NA>        log_Fmort                vector       4
       26 Fleet               1 <NA>        log_Fmort                vector       4
       27 Fleet               1 <NA>        log_Fmort                vector       4
       28 Fleet               1 <NA>        log_Fmort                vector       4
       29 Fleet               1 <NA>        log_Fmort                vector       4
       30 Fleet               1 <NA>        log_Fmort                vector       4
       31 Fleet               1 <NA>        log_q                    vector       3
       32 Fleet               1 <NA>        age_to_length_conversion vector      11
       33 Fleet               1 <NA>        age_to_length_conversion vector      11
       34 Fleet               1 <NA>        age_to_length_conversion vector      11
       35 Fleet               1 <NA>        age_to_length_conversion vector      11
       36 Fleet               1 <NA>        age_to_length_conversion vector      11
       37 Fleet               1 <NA>        age_to_length_conversion vector      11
       38 Fleet               1 <NA>        age_to_length_conversion vector      11
       39 Fleet               1 <NA>        age_to_length_conversion vector      11
       40 Fleet               1 <NA>        age_to_length_conversion vector      11
       41 Fleet               1 <NA>        age_to_length_conversion vector      11
       42 Fleet               1 <NA>        age_to_length_conversion vector      11
       43 Fleet               1 <NA>        age_to_length_conversion vector      11
       44 Fleet               1 <NA>        age_to_length_conversion vector      11
       45 Fleet               1 <NA>        age_to_length_conversion vector      11
       46 Fleet               1 <NA>        age_to_length_conversion vector      11
       47 Fleet               1 <NA>        age_to_length_conversion vector      11
       48 Fleet               1 <NA>        age_to_length_conversion vector      11
       49 Fleet               1 <NA>        age_to_length_conversion vector      11
       50 Fleet               1 <NA>        age_to_length_conversion vector      11
       51 Fleet               1 <NA>        age_to_length_conversion vector      11
       52 Fleet               1 <NA>        age_to_length_conversion vector      11
       53 Fleet               1 <NA>        age_to_length_conversion vector      11
       54 Fleet               1 <NA>        age_to_length_conversion vector      11
       55 Fleet               1 <NA>        age_to_length_conversion vector      11
       56 Fleet               1 <NA>        age_to_length_conversion vector      11
       57 Fleet               1 <NA>        age_to_length_conversion vector      11
       58 Fleet               1 <NA>        age_to_length_conversion vector      11
       59 Fleet               1 <NA>        age_to_length_conversion vector      11
       60 Fleet               1 <NA>        age_to_length_conversion vector      11
       61 Fleet               1 <NA>        age_to_length_conversion vector      11
       62 Fleet               1 <NA>        age_to_length_conversion vector      11
       63 Fleet               1 <NA>        age_to_length_conversion vector      11
       64 Fleet               1 <NA>        age_to_length_conversion vector      11
       65 Fleet               1 <NA>        age_to_length_conversion vector      11
       66 Fleet               1 <NA>        age_to_length_conversion vector      11
       67 Fleet               1 <NA>        age_to_length_conversion vector      11
       68 Fleet               1 <NA>        age_to_length_conversion vector      11
       69 Fleet               1 <NA>        age_to_length_conversion vector      11
       70 Fleet               1 <NA>        age_to_length_conversion vector      11
       71 Fleet               1 <NA>        age_to_length_conversion vector      11
       72 Fleet               1 <NA>        age_to_length_conversion vector      11
       73 Fleet               1 <NA>        age_to_length_conversion vector      11
       74 Fleet               1 <NA>        age_to_length_conversion vector      11
       75 Fleet               1 <NA>        age_to_length_conversion vector      11
       76 Fleet               1 <NA>        age_to_length_conversion vector      11
       77 Fleet               1 <NA>        age_to_length_conversion vector      11
       78 Fleet               1 <NA>        age_to_length_conversion vector      11
       79 Fleet               1 <NA>        age_to_length_conversion vector      11
       80 Fleet               1 <NA>        age_to_length_conversion vector      11
       81 Fleet               1 <NA>        age_to_length_conversion vector      11
       82 Fleet               1 <NA>        age_to_length_conversion vector      11
       83 Fleet               1 <NA>        age_to_length_conversion vector      11
       84 Fleet               1 <NA>        age_to_length_conversion vector      11
       85 Fleet               1 <NA>        age_to_length_conversion vector      11
       86 Fleet               1 <NA>        age_to_length_conversion vector      11
       87 Fleet               1 <NA>        age_to_length_conversion vector      11
       88 Fleet               1 <NA>        age_to_length_conversion vector      11
       89 Fleet               1 <NA>        age_to_length_conversion vector      11
       90 Fleet               1 <NA>        age_to_length_conversion vector      11
       91 Fleet               1 <NA>        age_to_length_conversion vector      11
       92 Fleet               1 <NA>        age_to_length_conversion vector      11
       93 Fleet               1 <NA>        age_to_length_conversion vector      11
       94 Fleet               1 <NA>        age_to_length_conversion vector      11
       95 Fleet               1 <NA>        age_to_length_conversion vector      11
       96 Fleet               1 <NA>        age_to_length_conversion vector      11
       97 Fleet               1 <NA>        age_to_length_conversion vector      11
       98 Fleet               1 <NA>        age_to_length_conversion vector      11
       99 Fleet               1 <NA>        age_to_length_conversion vector      11
      100 Fleet               1 <NA>        age_to_length_conversion vector      11
      101 Fleet               1 <NA>        age_to_length_conversion vector      11
      102 Fleet               1 <NA>        age_to_length_conversion vector      11
      103 Fleet               1 <NA>        age_to_length_conversion vector      11
      104 Fleet               1 <NA>        age_to_length_conversion vector      11
      105 Fleet               1 <NA>        age_to_length_conversion vector      11
      106 Fleet               1 <NA>        age_to_length_conversion vector      11
      107 Fleet               1 <NA>        age_to_length_conversion vector      11
      108 Fleet               1 <NA>        age_to_length_conversion vector      11
      109 Fleet               1 <NA>        age_to_length_conversion vector      11
      110 Fleet               1 <NA>        age_to_length_conversion vector      11
      111 Fleet               1 <NA>        age_to_length_conversion vector      11
      112 Fleet               1 <NA>        age_to_length_conversion vector      11
      113 Fleet               1 <NA>        age_to_length_conversion vector      11
      114 Fleet               1 <NA>        age_to_length_conversion vector      11
      115 Fleet               1 <NA>        age_to_length_conversion vector      11
      116 Fleet               1 <NA>        age_to_length_conversion vector      11
      117 Fleet               1 <NA>        age_to_length_conversion vector      11
      118 Fleet               1 <NA>        age_to_length_conversion vector      11
      119 Fleet               1 <NA>        age_to_length_conversion vector      11
      120 Fleet               1 <NA>        age_to_length_conversion vector      11
      121 Fleet               1 <NA>        age_to_length_conversion vector      11
      122 Fleet               1 <NA>        age_to_length_conversion vector      11
      123 Fleet               1 <NA>        age_to_length_conversion vector      11
      124 Fleet               1 <NA>        age_to_length_conversion vector      11
      125 Fleet               1 <NA>        age_to_length_conversion vector      11
      126 Fleet               1 <NA>        age_to_length_conversion vector      11
      127 Fleet               1 <NA>        age_to_length_conversion vector      11
      128 Fleet               1 <NA>        age_to_length_conversion vector      11
      129 Fleet               1 <NA>        age_to_length_conversion vector      11
      130 Fleet               1 <NA>        age_to_length_conversion vector      11
      131 Fleet               1 <NA>        age_to_length_conversion vector      11
      132 Fleet               1 <NA>        age_to_length_conversion vector      11
      133 Fleet               1 <NA>        age_to_length_conversion vector      11
      134 Fleet               1 <NA>        age_to_length_conversion vector      11
      135 Fleet               1 <NA>        age_to_length_conversion vector      11
      136 Fleet               1 <NA>        age_to_length_conversion vector      11
      137 Fleet               1 <NA>        age_to_length_conversion vector      11
      138 Fleet               1 <NA>        age_to_length_conversion vector      11
      139 Fleet               1 <NA>        age_to_length_conversion vector      11
      140 Fleet               1 <NA>        age_to_length_conversion vector      11
      141 Fleet               1 <NA>        age_to_length_conversion vector      11
      142 Fleet               1 <NA>        age_to_length_conversion vector      11
      143 Fleet               1 <NA>        age_to_length_conversion vector      11
      144 Fleet               1 <NA>        age_to_length_conversion vector      11
      145 Fleet               1 <NA>        age_to_length_conversion vector      11
      146 Fleet               1 <NA>        age_to_length_conversion vector      11
      147 Fleet               1 <NA>        age_to_length_conversion vector      11
      148 Fleet               1 <NA>        age_to_length_conversion vector      11
      149 Fleet               1 <NA>        age_to_length_conversion vector      11
      150 Fleet               1 <NA>        age_to_length_conversion vector      11
      151 Fleet               1 <NA>        age_to_length_conversion vector      11
      152 Fleet               1 <NA>        age_to_length_conversion vector      11
      153 Fleet               1 <NA>        age_to_length_conversion vector      11
      154 Fleet               1 <NA>        age_to_length_conversion vector      11
      155 Fleet               1 <NA>        age_to_length_conversion vector      11
      156 Fleet               1 <NA>        age_to_length_conversion vector      11
      157 Fleet               1 <NA>        age_to_length_conversion vector      11
      158 Fleet               1 <NA>        age_to_length_conversion vector      11
      159 Fleet               1 <NA>        age_to_length_conversion vector      11
      160 Fleet               1 <NA>        age_to_length_conversion vector      11
      161 Fleet               1 <NA>        age_to_length_conversion vector      11
      162 Fleet               1 <NA>        age_to_length_conversion vector      11
      163 Fleet               1 <NA>        age_to_length_conversion vector      11
      164 Fleet               1 <NA>        age_to_length_conversion vector      11
      165 Fleet               1 <NA>        age_to_length_conversion vector      11
      166 Fleet               1 <NA>        age_to_length_conversion vector      11
      167 Fleet               1 <NA>        age_to_length_conversion vector      11
      168 Fleet               1 <NA>        age_to_length_conversion vector      11
      169 Fleet               1 <NA>        age_to_length_conversion vector      11
      170 Fleet               1 <NA>        age_to_length_conversion vector      11
      171 Fleet               1 <NA>        age_to_length_conversion vector      11
      172 Fleet               1 <NA>        age_to_length_conversion vector      11
      173 Fleet               1 <NA>        age_to_length_conversion vector      11
      174 Fleet               1 <NA>        age_to_length_conversion vector      11
      175 Fleet               1 <NA>        age_to_length_conversion vector      11
      176 Fleet               1 <NA>        age_to_length_conversion vector      11
      177 Fleet               1 <NA>        age_to_length_conversion vector      11
      178 Fleet               1 <NA>        age_to_length_conversion vector      11
      179 Fleet               1 <NA>        age_to_length_conversion vector      11
      180 Fleet               1 <NA>        age_to_length_conversion vector      11
      181 Fleet               1 <NA>        age_to_length_conversion vector      11
      182 Fleet               1 <NA>        age_to_length_conversion vector      11
      183 Fleet               1 <NA>        age_to_length_conversion vector      11
      184 Fleet               1 <NA>        age_to_length_conversion vector      11
      185 Fleet               1 <NA>        age_to_length_conversion vector      11
      186 Fleet               1 <NA>        age_to_length_conversion vector      11
      187 Fleet               1 <NA>        age_to_length_conversion vector      11
      188 Fleet               1 <NA>        age_to_length_conversion vector      11
      189 Fleet               1 <NA>        age_to_length_conversion vector      11
      190 Fleet               1 <NA>        age_to_length_conversion vector      11
      191 Fleet               1 <NA>        age_to_length_conversion vector      11
      192 Fleet               1 <NA>        age_to_length_conversion vector      11
      193 Fleet               1 <NA>        age_to_length_conversion vector      11
      194 Fleet               1 <NA>        age_to_length_conversion vector      11
      195 Fleet               1 <NA>        age_to_length_conversion vector      11
      196 Fleet               1 <NA>        age_to_length_conversion vector      11
      197 Fleet               1 <NA>        age_to_length_conversion vector      11
      198 Fleet               1 <NA>        age_to_length_conversion vector      11
      199 Fleet               1 <NA>        age_to_length_conversion vector      11
      200 Fleet               1 <NA>        age_to_length_conversion vector      11
      201 Fleet               1 <NA>        age_to_length_conversion vector      11
      202 Fleet               1 <NA>        age_to_length_conversion vector      11
      203 Fleet               1 <NA>        age_to_length_conversion vector      11
      204 Fleet               1 <NA>        age_to_length_conversion vector      11
      205 Fleet               1 <NA>        age_to_length_conversion vector      11
      206 Fleet               1 <NA>        age_to_length_conversion vector      11
      207 Fleet               1 <NA>        age_to_length_conversion vector      11
      208 Fleet               1 <NA>        age_to_length_conversion vector      11
      209 Fleet               1 <NA>        age_to_length_conversion vector      11
      210 Fleet               1 <NA>        age_to_length_conversion vector      11
      211 Fleet               1 <NA>        age_to_length_conversion vector      11
      212 Fleet               1 <NA>        age_to_length_conversion vector      11
      213 Fleet               1 <NA>        age_to_length_conversion vector      11
      214 Fleet               1 <NA>        age_to_length_conversion vector      11
      215 Fleet               1 <NA>        age_to_length_conversion vector      11
      216 Fleet               1 <NA>        age_to_length_conversion vector      11
      217 Fleet               1 <NA>        age_to_length_conversion vector      11
      218 Fleet               1 <NA>        age_to_length_conversion vector      11
      219 Fleet               1 <NA>        age_to_length_conversion vector      11
      220 Fleet               1 <NA>        age_to_length_conversion vector      11
      221 Fleet               1 <NA>        age_to_length_conversion vector      11
      222 Fleet               1 <NA>        age_to_length_conversion vector      11
      223 Fleet               1 <NA>        age_to_length_conversion vector      11
      224 Fleet               1 <NA>        age_to_length_conversion vector      11
      225 Fleet               1 <NA>        age_to_length_conversion vector      11
      226 Fleet               1 <NA>        age_to_length_conversion vector      11
      227 Fleet               1 <NA>        age_to_length_conversion vector      11
      228 Fleet               1 <NA>        age_to_length_conversion vector      11
      229 Fleet               1 <NA>        age_to_length_conversion vector      11
      230 Fleet               1 <NA>        age_to_length_conversion vector      11
      231 Fleet               1 <NA>        age_to_length_conversion vector      11
      232 Fleet               1 <NA>        age_to_length_conversion vector      11
      233 Fleet               1 <NA>        age_to_length_conversion vector      11
      234 Fleet               1 <NA>        age_to_length_conversion vector      11
      235 Fleet               1 <NA>        age_to_length_conversion vector      11
      236 Fleet               1 <NA>        age_to_length_conversion vector      11
      237 Fleet               1 <NA>        age_to_length_conversion vector      11
      238 Fleet               1 <NA>        age_to_length_conversion vector      11
      239 Fleet               1 <NA>        age_to_length_conversion vector      11
      240 Fleet               1 <NA>        age_to_length_conversion vector      11
      241 Fleet               1 <NA>        age_to_length_conversion vector      11
      242 Fleet               1 <NA>        age_to_length_conversion vector      11
      243 Fleet               1 <NA>        age_to_length_conversion vector      11
      244 Fleet               1 <NA>        age_to_length_conversion vector      11
      245 Fleet               1 <NA>        age_to_length_conversion vector      11
      246 Fleet               1 <NA>        age_to_length_conversion vector      11
      247 Fleet               1 <NA>        age_to_length_conversion vector      11
      248 Fleet               1 <NA>        age_to_length_conversion vector      11
      249 Fleet               1 <NA>        age_to_length_conversion vector      11
      250 Fleet               1 <NA>        age_to_length_conversion vector      11
      251 Fleet               1 <NA>        age_to_length_conversion vector      11
      252 Fleet               1 <NA>        age_to_length_conversion vector      11
      253 Fleet               1 <NA>        age_to_length_conversion vector      11
      254 Fleet               1 <NA>        age_to_length_conversion vector      11
      255 Fleet               1 <NA>        age_to_length_conversion vector      11
      256 Fleet               1 <NA>        age_to_length_conversion vector      11
      257 Fleet               1 <NA>        age_to_length_conversion vector      11
      258 Fleet               1 <NA>        age_to_length_conversion vector      11
      259 Fleet               1 <NA>        age_to_length_conversion vector      11
      260 Fleet               1 <NA>        age_to_length_conversion vector      11
      261 Fleet               1 <NA>        age_to_length_conversion vector      11
      262 Fleet               1 <NA>        age_to_length_conversion vector      11
      263 Fleet               1 <NA>        age_to_length_conversion vector      11
      264 Fleet               1 <NA>        age_to_length_conversion vector      11
      265 Fleet               1 <NA>        age_to_length_conversion vector      11
      266 Fleet               1 <NA>        age_to_length_conversion vector      11
      267 Fleet               1 <NA>        age_to_length_conversion vector      11
      268 Fleet               1 <NA>        age_to_length_conversion vector      11
      269 Fleet               1 <NA>        age_to_length_conversion vector      11
      270 Fleet               1 <NA>        age_to_length_conversion vector      11
      271 Fleet               1 <NA>        age_to_length_conversion vector      11
      272 Fleet               1 <NA>        age_to_length_conversion vector      11
      273 Fleet               1 <NA>        age_to_length_conversion vector      11
      274 Fleet               1 <NA>        age_to_length_conversion vector      11
      275 Fleet               1 <NA>        age_to_length_conversion vector      11
      276 Fleet               1 <NA>        age_to_length_conversion vector      11
      277 Fleet               1 <NA>        age_to_length_conversion vector      11
      278 Fleet               1 <NA>        age_to_length_conversion vector      11
      279 Fleet               1 <NA>        age_to_length_conversion vector      11
      280 Fleet               1 <NA>        age_to_length_conversion vector      11
      281 Fleet               1 <NA>        age_to_length_conversion vector      11
      282 Fleet               1 <NA>        age_to_length_conversion vector      11
      283 Fleet               1 <NA>        age_to_length_conversion vector      11
      284 Fleet               1 <NA>        age_to_length_conversion vector      11
      285 Fleet               1 <NA>        age_to_length_conversion vector      11
      286 Fleet               1 <NA>        age_to_length_conversion vector      11
      287 Fleet               1 <NA>        age_to_length_conversion vector      11
      288 Fleet               1 <NA>        age_to_length_conversion vector      11
      289 Fleet               1 <NA>        age_to_length_conversion vector      11
      290 Fleet               1 <NA>        age_to_length_conversion vector      11
      291 Fleet               1 <NA>        age_to_length_conversion vector      11
      292 Fleet               1 <NA>        age_to_length_conversion vector      11
      293 Fleet               1 <NA>        age_to_length_conversion vector      11
      294 Fleet               1 <NA>        age_to_length_conversion vector      11
      295 Fleet               1 <NA>        age_to_length_conversion vector      11
      296 Fleet               1 <NA>        age_to_length_conversion vector      11
      297 Fleet               1 <NA>        age_to_length_conversion vector      11
      298 Fleet               1 <NA>        age_to_length_conversion vector      11
      299 Fleet               1 <NA>        age_to_length_conversion vector      11
      300 Fleet               1 <NA>        age_to_length_conversion vector      11
      301 Fleet               1 <NA>        age_to_length_conversion vector      11
      302 Fleet               1 <NA>        age_to_length_conversion vector      11
      303 Fleet               1 <NA>        age_to_length_conversion vector      11
      304 Fleet               1 <NA>        age_to_length_conversion vector      11
      305 Fleet               1 <NA>        age_to_length_conversion vector      11
      306 Fleet               1 <NA>        age_to_length_conversion vector      11
      307 Fleet               1 <NA>        age_to_length_conversion vector      11
      308 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      309 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      310 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      311 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      312 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      313 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      314 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      315 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      316 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      317 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      318 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      319 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      320 Fleet               1 <NA>        agecomp_expected         <NA>        NA
          parameter_id fleet year_i age_i length_i     input observed estimation_type 
                 <int> <chr>  <int> <int>    <int>     <dbl>    <dbl> <chr>           
        1            4 NA         1    NA       NA -4.66e+ 0       NA fixed_effects   
        2          287 NA         2    NA       NA -3.60e+ 0       NA fixed_effects   
        3          288 NA         3    NA       NA -3.10e+ 0       NA fixed_effects   
        4          289 NA         4    NA       NA -2.80e+ 0       NA fixed_effects   
        5          290 NA         5    NA       NA -3.02e+ 0       NA fixed_effects   
        6          291 NA         6    NA       NA -2.44e+ 0       NA fixed_effects   
        7          292 NA         7    NA       NA -2.43e+ 0       NA fixed_effects   
        8          293 NA         8    NA       NA -1.68e+ 0       NA fixed_effects   
        9          294 NA         9    NA       NA -2.22e+ 0       NA fixed_effects   
       10          295 NA        10    NA       NA -2.02e+ 0       NA fixed_effects   
       11          296 NA        11    NA       NA -1.89e+ 0       NA fixed_effects   
       12          297 NA        12    NA       NA -1.82e+ 0       NA fixed_effects   
       13          298 NA        13    NA       NA -2.15e+ 0       NA fixed_effects   
       14          299 NA        14    NA       NA -1.78e+ 0       NA fixed_effects   
       15          300 NA        15    NA       NA -1.71e+ 0       NA fixed_effects   
       16          301 NA        16    NA       NA -1.82e+ 0       NA fixed_effects   
       17          302 NA        17    NA       NA -1.16e+ 0       NA fixed_effects   
       18          303 NA        18    NA       NA -1.36e+ 0       NA fixed_effects   
       19          304 NA        19    NA       NA -1.37e+ 0       NA fixed_effects   
       20          305 NA        20    NA       NA -1.38e+ 0       NA fixed_effects   
       21          306 NA        21    NA       NA -1.05e+ 0       NA fixed_effects   
       22          307 NA        22    NA       NA -1.37e+ 0       NA fixed_effects   
       23          308 NA        23    NA       NA -8.71e- 1       NA fixed_effects   
       24          309 NA        24    NA       NA -1.06e+ 0       NA fixed_effects   
       25          310 NA        25    NA       NA -1.07e+ 0       NA fixed_effects   
       26          311 NA        26    NA       NA -1.16e+ 0       NA fixed_effects   
       27          312 NA        27    NA       NA -1.18e+ 0       NA fixed_effects   
       28          313 NA        28    NA       NA -8.40e- 1       NA fixed_effects   
       29          314 NA        29    NA       NA -1.11e+ 0       NA fixed_effects   
       30          315 NA        30    NA       NA -6.94e- 1       NA fixed_effects   
       31            3 NA        NA    NA       NA  0              NA constant        
       32           11 NA        NA     1        1  1.26e-16       NA constant        
       33           12 NA        NA     1        2  8.39e-11       NA constant        
       34           13 NA        NA     1        3  2.30e- 6       NA constant        
       35           14 NA        NA     1        4  2.74e- 3       NA constant        
       36           15 NA        NA     1        5  1.63e- 1       NA constant        
       37           16 NA        NA     1        6  6.32e- 1       NA constant        
       38           17 NA        NA     1        7  1.98e- 1       NA constant        
       39           18 NA        NA     1        8  4.13e- 3       NA constant        
       40           19 NA        NA     1        9  4.36e- 6       NA constant        
       41           20 NA        NA     1       10  2.02e-10       NA constant        
       42           21 NA        NA     1       11  3.33e-16       NA constant        
       43           22 NA        NA     1       12  0              NA constant        
       44           23 NA        NA     1       13  0              NA constant        
       45           24 NA        NA     1       14  0              NA constant        
       46           25 NA        NA     1       15  0              NA constant        
       47           26 NA        NA     1       16  0              NA constant        
       48           27 NA        NA     1       17  0              NA constant        
       49           28 NA        NA     1       18  0              NA constant        
       50           29 NA        NA     1       19  0              NA constant        
       51           30 NA        NA     1       20  0              NA constant        
       52           31 NA        NA     1       21  0              NA constant        
       53           32 NA        NA     1       22  0              NA constant        
       54           33 NA        NA     1       23  0              NA constant        
       55           34 NA        NA     2        1  3.27e-18       NA constant        
       56           35 NA        NA     2        2  2.15e-13       NA constant        
       57           36 NA        NA     2        3  2.20e- 9       NA constant        
       58           37 NA        NA     2        4  3.54e- 6       NA constant        
       59           38 NA        NA     2        5  9.19e- 4       NA constant        
       60           39 NA        NA     2        6  4.03e- 2       NA constant        
       61           40 NA        NA     2        7  3.18e- 1       NA constant        
       62           41 NA        NA     2        8  4.86e- 1       NA constant        
       63           42 NA        NA     2        9  1.46e- 1       NA constant        
       64           43 NA        NA     2       10  8.23e- 3       NA constant        
       65           44 NA        NA     2       11  8.08e- 5       NA constant        
       66           45 NA        NA     2       12  1.31e- 7       NA constant        
       67           46 NA        NA     2       13  3.37e-11       NA constant        
       68           47 NA        NA     2       14  1.33e-15       NA constant        
       69           48 NA        NA     2       15  0              NA constant        
       70           49 NA        NA     2       16  0              NA constant        
       71           50 NA        NA     2       17  0              NA constant        
       72           51 NA        NA     2       18  0              NA constant        
       73           52 NA        NA     2       19  0              NA constant        
       74           53 NA        NA     2       20  0              NA constant        
       75           54 NA        NA     2       21  0              NA constant        
       76           55 NA        NA     2       22  0              NA constant        
       77           56 NA        NA     2       23  0              NA constant        
       78           57 NA        NA     3        1  4.35e-19       NA constant        
       79           58 NA        NA     3        2  6.73e-15       NA constant        
       80           59 NA        NA     3        3  2.84e-11       NA constant        
       81           60 NA        NA     3        4  3.28e- 8       NA constant        
       82           61 NA        NA     3        5  1.05e- 5       NA constant        
       83           62 NA        NA     3        6  9.44e- 4       NA constant        
       84           63 NA        NA     3        7  2.44e- 2       NA constant        
       85           64 NA        NA     3        8  1.85e- 1       NA constant        
       86           65 NA        NA     3        9  4.24e- 1       NA constant        
       87           66 NA        NA     3       10  2.98e- 1       NA constant        
       88           67 NA        NA     3       11  6.36e- 2       NA constant        
       89           68 NA        NA     3       12  4.04e- 3       NA constant        
       90           69 NA        NA     3       13  7.43e- 5       NA constant        
       91           70 NA        NA     3       14  3.87e- 7       NA constant        
       92           71 NA        NA     3       15  5.62e-10       NA constant        
       93           72 NA        NA     3       16  2.24e-13       NA constant        
       94           73 NA        NA     3       17  0              NA constant        
       95           74 NA        NA     3       18  0              NA constant        
       96           75 NA        NA     3       19  0              NA constant        
       97           76 NA        NA     3       20  0              NA constant        
       98           77 NA        NA     3       21  0              NA constant        
       99           78 NA        NA     3       22  0              NA constant        
      100           79 NA        NA     3       23  0              NA constant        
      101           80 NA        NA     4        1  1.23e-19       NA constant        
      102           81 NA        NA     4        2  7.29e-16       NA constant        
      103           82 NA        NA     4        3  1.58e-12       NA constant        
      104           83 NA        NA     4        4  1.25e- 9       NA constant        
      105           84 NA        NA     4        5  3.68e- 7       NA constant        
      106           85 NA        NA     4        6  4.02e- 5       NA constant        
      107           86 NA        NA     4        7  1.65e- 3       NA constant        
      108           87 NA        NA     4        8  2.56e- 2       NA constant        
      109           88 NA        NA     4        9  1.54e- 1       NA constant        
      110           89 NA        NA     4       10  3.58e- 1       NA constant        
      111           90 NA        NA     4       11  3.27e- 1       NA constant        
      112           91 NA        NA     4       12  1.17e- 1       NA constant        
      113           92 NA        NA     4       13  1.62e- 2       NA constant        
      114           93 NA        NA     4       14  8.65e- 4       NA constant        
      115           94 NA        NA     4       15  1.75e- 5       NA constant        
      116           95 NA        NA     4       16  1.32e- 7       NA constant        
      117           96 NA        NA     4       17  3.71e-10       NA constant        
      118           97 NA        NA     4       18  3.85e-13       NA constant        
      119           98 NA        NA     4       19  1.11e-16       NA constant        
      120           99 NA        NA     4       20  0              NA constant        
      121          100 NA        NA     4       21  0              NA constant        
      122          101 NA        NA     4       22  0              NA constant        
      123          102 NA        NA     4       23  0              NA constant        
      124          103 NA        NA     5        1  5.27e-20       NA constant        
      125          104 NA        NA     5        2  1.59e-16       NA constant        
      126          105 NA        NA     5        3  2.09e-13       NA constant        
      127          106 NA        NA     5        4  1.20e-10       NA constant        
      128          107 NA        NA     5        5  3.04e- 8       NA constant        
      129          108 NA        NA     5        6  3.38e- 6       NA constant        
      130          109 NA        NA     5        7  1.67e- 4       NA constant        
      131          110 NA        NA     5        8  3.67e- 3       NA constant        
      132          111 NA        NA     5        9  3.63e- 2       NA constant        
      133          112 NA        NA     5       10  1.63e- 1       NA constant        
      134          113 NA        NA     5       11  3.31e- 1       NA constant        
      135          114 NA        NA     5       12  3.08e- 1       NA constant        
      136          115 NA        NA     5       13  1.31e- 1       NA constant        
      137          116 NA        NA     5       14  2.52e- 2       NA constant        
      138          117 NA        NA     5       15  2.20e- 3       NA constant        
      139          118 NA        NA     5       16  8.62e- 5       NA constant        
      140          119 NA        NA     5       17  1.50e- 6       NA constant        
      141          120 NA        NA     5       18  1.16e- 8       NA constant        
      142          121 NA        NA     5       19  3.94e-11       NA constant        
      143          122 NA        NA     5       20  5.88e-14       NA constant        
      144          123 NA        NA     5       21  0              NA constant        
      145          124 NA        NA     5       22  0              NA constant        
      146          125 NA        NA     5       23  0              NA constant        
      147          126 NA        NA     6        1  2.88e-20       NA constant        
      148          127 NA        NA     6        2  5.32e-17       NA constant        
      149          128 NA        NA     6        3  4.81e-14       NA constant        
      150          129 NA        NA     6        4  2.13e-11       NA constant        
      151          130 NA        NA     6        5  4.62e- 9       NA constant        
      152          131 NA        NA     6        6  4.94e- 7       NA constant        
      153          132 NA        NA     6        7  2.61e- 5       NA constant        
      154          133 NA        NA     6        8  6.86e- 4       NA constant        
      155          134 NA        NA     6        9  8.98e- 3       NA constant        
      156          135 NA        NA     6       10  5.89e- 2       NA constant        
      157          136 NA        NA     6       11  1.94e- 1       NA constant        
      158          137 NA        NA     6       12  3.23e- 1       NA constant        
      159          138 NA        NA     6       13  2.72e- 1       NA constant        
      160          139 NA        NA     6       14  1.15e- 1       NA constant        
      161          140 NA        NA     6       15  2.47e- 2       NA constant        
      162          141 NA        NA     6       16  2.66e- 3       NA constant        
      163          142 NA        NA     6       17  1.43e- 4       NA constant        
      164          143 NA        NA     6       18  3.81e- 6       NA constant        
      165          144 NA        NA     6       19  5.04e- 8       NA constant        
      166          145 NA        NA     6       20  3.29e-10       NA constant        
      167          146 NA        NA     6       21  1.06e-12       NA constant        
      168          147 NA        NA     6       22  1.67e-15       NA constant        
      169          148 NA        NA     6       23  0              NA constant        
      170          149 NA        NA     7        1  1.85e-20       NA constant        
      171          150 NA        NA     7        2  2.36e-17       NA constant        
      172          151 NA        NA     7        3  1.60e-14       NA constant        
      173          152 NA        NA     7        4  5.73e-12       NA constant        
      174          153 NA        NA     7        5  1.09e- 9       NA constant        
      175          154 NA        NA     7        6  1.10e- 7       NA constant        
      176          155 NA        NA     7        7  5.93e- 6       NA constant        
      177          156 NA        NA     7        8  1.71e- 4       NA constant        
      178          157 NA        NA     7        9  2.63e- 3       NA constant        
      179          158 NA        NA     7       10  2.18e- 2       NA constant        
      180          159 NA        NA     7       11  9.79e- 2       NA constant        
      181          160 NA        NA     7       12  2.37e- 1       NA constant        
      182          161 NA        NA     7       13  3.12e- 1       NA constant        
      183          162 NA        NA     7       14  2.22e- 1       NA constant        
      184          163 NA        NA     7       15  8.59e- 2       NA constant        
      185          164 NA        NA     7       16  1.80e- 2       NA constant        
      186          165 NA        NA     7       17  2.03e- 3       NA constant        
      187          166 NA        NA     7       18  1.23e- 4       NA constant        
      188          167 NA        NA     7       19  4.00e- 6       NA constant        
      189          168 NA        NA     7       20  6.96e- 8       NA constant        
      190          169 NA        NA     7       21  6.44e-10       NA constant        
      191          170 NA        NA     7       22  3.17e-12       NA constant        
      192          171 NA        NA     7       23  8.33e-15       NA constant        
      193          172 NA        NA     8        1  1.32e-20       NA constant        
      194          173 NA        NA     8        2  1.28e-17       NA constant        
      195          174 NA        NA     8        3  6.90e-15       NA constant        
      196          175 NA        NA     8        4  2.09e-12       NA constant        
      197          176 NA        NA     8        5  3.54e-10       NA constant        
      198          177 NA        NA     8        6  3.37e- 8       NA constant        
      199          178 NA        NA     8        7  1.81e- 6       NA constant        
      200          179 NA        NA     8        8  5.45e- 5       NA constant        
      201          180 NA        NA     8        9  9.31e- 4       NA constant        
      202          181 NA        NA     8       10  9.00e- 3       NA constant        
      203          182 NA        NA     8       11  4.95e- 2       NA constant        
      204          183 NA        NA     8       12  1.55e- 1       NA constant        
      205          184 NA        NA     8       13  2.76e- 1       NA constant        
      206          185 NA        NA     8       14  2.81e- 1       NA constant        
      207          186 NA        NA     8       15  1.63e- 1       NA constant        
      208          187 NA        NA     8       16  5.42e- 2       NA constant        
      209          188 NA        NA     8       17  1.02e- 2       NA constant        
      210          189 NA        NA     8       18  1.10e- 3       NA constant        
      211          190 NA        NA     8       19  6.66e- 5       NA constant        
      212          191 NA        NA     8       20  2.29e- 6       NA constant        
      213          192 NA        NA     8       21  4.44e- 8       NA constant        
      214          193 NA        NA     8       22  4.83e-10       NA constant        
      215          194 NA        NA     8       23  2.97e-12       NA constant        
      216          195 NA        NA     9        1  1.02e-20       NA constant        
      217          196 NA        NA     9        2  7.91e-18       NA constant        
      218          197 NA        NA     9        3  3.58e-15       NA constant        
      219          198 NA        NA     9        4  9.46e-13       NA constant        
      220          199 NA        NA     9        5  1.46e-10       NA constant        
      221          200 NA        NA     9        6  1.31e- 8       NA constant        
      222          201 NA        NA     9        7  6.94e- 7       NA constant        
      223          202 NA        NA     9        8  2.15e- 5       NA constant        
      224          203 NA        NA     9        9  3.91e- 4       NA constant        
      225          204 NA        NA     9       10  4.19e- 3       NA constant        
      226          205 NA        NA     9       11  2.65e- 2       NA constant        
      227          206 NA        NA     9       12  9.92e- 2       NA constant        
      228          207 NA        NA     9       13  2.20e- 1       NA constant        
      229          208 NA        NA     9       14  2.88e- 1       NA constant        
      230          209 NA        NA     9       15  2.25e- 1       NA constant        
      231          210 NA        NA     9       16  1.04e- 1       NA constant        
      232          211 NA        NA     9       17  2.83e- 2       NA constant        
      233          212 NA        NA     9       18  4.58e- 3       NA constant        
      234          213 NA        NA     9       19  4.37e- 4       NA constant        
      235          214 NA        NA     9       20  2.46e- 5       NA constant        
      236          215 NA        NA     9       21  8.11e- 7       NA constant        
      237          216 NA        NA     9       22  1.57e- 8       NA constant        
      238          217 NA        NA     9       23  1.80e-10       NA constant        
      239          218 NA        NA    10        1  8.31e-21       NA constant        
      240          219 NA        NA    10        2  5.43e-18       NA constant        
      241          220 NA        NA    10        3  2.14e-15       NA constant        
      242          221 NA        NA    10        4  5.06e-13       NA constant        
      243          222 NA        NA    10        5  7.20e-11       NA constant        
      244          223 NA        NA    10        6  6.18e- 9       NA constant        
      245          224 NA        NA    10        7  3.20e- 7       NA constant        
      246          225 NA        NA    10        8  1.00e- 5       NA constant        
      247          226 NA        NA    10        9  1.91e- 4       NA constant        
      248          227 NA        NA    10       10  2.19e- 3       NA constant        
      249          228 NA        NA    10       11  1.53e- 2       NA constant        
      250          229 NA        NA    10       12  6.53e- 2       NA constant        
      251          230 NA        NA    10       13  1.69e- 1       NA constant        
      252          231 NA        NA    10       14  2.68e- 1       NA constant        
      253          232 NA        NA    10       15  2.59e- 1       NA constant        
      254          233 NA        NA    10       16  1.53e- 1       NA constant        
      255          234 NA        NA    10       17  5.48e- 2       NA constant        
      256          235 NA        NA    10       18  1.20e- 2       NA constant        
      257          236 NA        NA    10       19  1.60e- 3       NA constant        
      258          237 NA        NA    10       20  1.30e- 4       NA constant        
      259          238 NA        NA    10       21  6.37e- 6       NA constant        
      260          239 NA        NA    10       22  1.89e- 7       NA constant        
      261          240 NA        NA    10       23  3.44e- 9       NA constant        
      262          241 NA        NA    11        1  7.07e-21       NA constant        
      263          242 NA        NA    11        2  4.03e-18       NA constant        
      264          243 NA        NA    11        3  1.42e-15       NA constant        
      265          244 NA        NA    11        4  3.06e-13       NA constant        
      266          245 NA        NA    11        5  4.09e-11       NA constant        
      267          246 NA        NA    11        6  3.36e- 9       NA constant        
      268          247 NA        NA    11        7  1.71e- 7       NA constant        
      269          248 NA        NA    11        8  5.39e- 6       NA constant        
      270          249 NA        NA    11        9  1.05e- 4       NA constant        
      271          250 NA        NA    11       10  1.27e- 3       NA constant        
      272          251 NA        NA    11       11  9.57e- 3       NA constant        
      273          252 NA        NA    11       12  4.48e- 2       NA constant        
      274          253 NA        NA    11       13  1.31e- 1       NA constant        
      275          254 NA        NA    11       14  2.38e- 1       NA constant        
      276          255 NA        NA    11       15  2.71e- 1       NA constant        
      277          256 NA        NA    11       16  1.92e- 1       NA constant        
      278          257 NA        NA    11       17  8.49e- 2       NA constant        
      279          258 NA        NA    11       18  2.34e- 2       NA constant        
      280          259 NA        NA    11       19  4.02e- 3       NA constant        
      281          260 NA        NA    11       20  4.30e- 4       NA constant        
      282          261 NA        NA    11       21  2.85e- 5       NA constant        
      283          262 NA        NA    11       22  1.17e- 6       NA constant        
      284          263 NA        NA    11       23  3.04e- 8       NA constant        
      285          264 NA        NA    12        1  6.22e-21       NA constant        
      286          265 NA        NA    12        2  3.17e-18       NA constant        
      287          266 NA        NA    12        3  1.02e-15       NA constant        
      288          267 NA        NA    12        4  2.04e-13       NA constant        
      289          268 NA        NA    12        5  2.58e-11       NA constant        
      290          269 NA        NA    12        6  2.05e- 9       NA constant        
      291          270 NA        NA    12        7  1.03e- 7       NA constant        
      292          271 NA        NA    12        8  3.23e- 6       NA constant        
      293          272 NA        NA    12        9  6.43e- 5       NA constant        
      294          273 NA        NA    12       10  8.07e- 4       NA constant        
      295          274 NA        NA    12       11  6.41e- 3       NA constant        
      296          275 NA        NA    12       12  3.23e- 2       NA constant        
      297          276 NA        NA    12       13  1.03e- 1       NA constant        
      298          277 NA        NA    12       14  2.09e- 1       NA constant        
      299          278 NA        NA    12       15  2.69e- 1       NA constant        
      300          279 NA        NA    12       16  2.20e- 1       NA constant        
      301          280 NA        NA    12       17  1.14e- 1       NA constant        
      302          281 NA        NA    12       18  3.76e- 2       NA constant        
      303          282 NA        NA    12       19  7.85e- 3       NA constant        
      304          283 NA        NA    12       20  1.04e- 3       NA constant        
      305          284 NA        NA    12       21  8.70e- 5       NA constant        
      306          285 NA        NA    12       22  4.61e- 6       NA constant        
      307          286 NA        NA    12       23  1.57e- 7       NA constant        
      308           NA NA         1     1       NA NA              14 derived_quantity
      309           NA NA         1     2       NA NA              20 derived_quantity
      310           NA NA         1     3       NA NA              23 derived_quantity
      311           NA NA         1     4       NA NA              30 derived_quantity
      312           NA NA         1     5       NA NA              20 derived_quantity
      313           NA NA         1     6       NA NA              10 derived_quantity
      314           NA NA         1     7       NA NA              22 derived_quantity
      315           NA NA         1     8       NA NA              11 derived_quantity
      316           NA NA         1     9       NA NA              12 derived_quantity
      317           NA NA         1    10       NA NA               6 derived_quantity
      318           NA NA         1    11       NA NA               7 derived_quantity
      319           NA NA         1    12       NA NA              25 derived_quantity
      320           NA NA         2     1       NA NA              21 derived_quantity
          distribution input_type  lpdf
          <chr>        <chr>      <dbl>
        1 <NA>         <NA>         NA 
        2 <NA>         <NA>         NA 
        3 <NA>         <NA>         NA 
        4 <NA>         <NA>         NA 
        5 <NA>         <NA>         NA 
        6 <NA>         <NA>         NA 
        7 <NA>         <NA>         NA 
        8 <NA>         <NA>         NA 
        9 <NA>         <NA>         NA 
       10 <NA>         <NA>         NA 
       11 <NA>         <NA>         NA 
       12 <NA>         <NA>         NA 
       13 <NA>         <NA>         NA 
       14 <NA>         <NA>         NA 
       15 <NA>         <NA>         NA 
       16 <NA>         <NA>         NA 
       17 <NA>         <NA>         NA 
       18 <NA>         <NA>         NA 
       19 <NA>         <NA>         NA 
       20 <NA>         <NA>         NA 
       21 <NA>         <NA>         NA 
       22 <NA>         <NA>         NA 
       23 <NA>         <NA>         NA 
       24 <NA>         <NA>         NA 
       25 <NA>         <NA>         NA 
       26 <NA>         <NA>         NA 
       27 <NA>         <NA>         NA 
       28 <NA>         <NA>         NA 
       29 <NA>         <NA>         NA 
       30 <NA>         <NA>         NA 
       31 <NA>         <NA>         NA 
       32 <NA>         <NA>         NA 
       33 <NA>         <NA>         NA 
       34 <NA>         <NA>         NA 
       35 <NA>         <NA>         NA 
       36 <NA>         <NA>         NA 
       37 <NA>         <NA>         NA 
       38 <NA>         <NA>         NA 
       39 <NA>         <NA>         NA 
       40 <NA>         <NA>         NA 
       41 <NA>         <NA>         NA 
       42 <NA>         <NA>         NA 
       43 <NA>         <NA>         NA 
       44 <NA>         <NA>         NA 
       45 <NA>         <NA>         NA 
       46 <NA>         <NA>         NA 
       47 <NA>         <NA>         NA 
       48 <NA>         <NA>         NA 
       49 <NA>         <NA>         NA 
       50 <NA>         <NA>         NA 
       51 <NA>         <NA>         NA 
       52 <NA>         <NA>         NA 
       53 <NA>         <NA>         NA 
       54 <NA>         <NA>         NA 
       55 <NA>         <NA>         NA 
       56 <NA>         <NA>         NA 
       57 <NA>         <NA>         NA 
       58 <NA>         <NA>         NA 
       59 <NA>         <NA>         NA 
       60 <NA>         <NA>         NA 
       61 <NA>         <NA>         NA 
       62 <NA>         <NA>         NA 
       63 <NA>         <NA>         NA 
       64 <NA>         <NA>         NA 
       65 <NA>         <NA>         NA 
       66 <NA>         <NA>         NA 
       67 <NA>         <NA>         NA 
       68 <NA>         <NA>         NA 
       69 <NA>         <NA>         NA 
       70 <NA>         <NA>         NA 
       71 <NA>         <NA>         NA 
       72 <NA>         <NA>         NA 
       73 <NA>         <NA>         NA 
       74 <NA>         <NA>         NA 
       75 <NA>         <NA>         NA 
       76 <NA>         <NA>         NA 
       77 <NA>         <NA>         NA 
       78 <NA>         <NA>         NA 
       79 <NA>         <NA>         NA 
       80 <NA>         <NA>         NA 
       81 <NA>         <NA>         NA 
       82 <NA>         <NA>         NA 
       83 <NA>         <NA>         NA 
       84 <NA>         <NA>         NA 
       85 <NA>         <NA>         NA 
       86 <NA>         <NA>         NA 
       87 <NA>         <NA>         NA 
       88 <NA>         <NA>         NA 
       89 <NA>         <NA>         NA 
       90 <NA>         <NA>         NA 
       91 <NA>         <NA>         NA 
       92 <NA>         <NA>         NA 
       93 <NA>         <NA>         NA 
       94 <NA>         <NA>         NA 
       95 <NA>         <NA>         NA 
       96 <NA>         <NA>         NA 
       97 <NA>         <NA>         NA 
       98 <NA>         <NA>         NA 
       99 <NA>         <NA>         NA 
      100 <NA>         <NA>         NA 
      101 <NA>         <NA>         NA 
      102 <NA>         <NA>         NA 
      103 <NA>         <NA>         NA 
      104 <NA>         <NA>         NA 
      105 <NA>         <NA>         NA 
      106 <NA>         <NA>         NA 
      107 <NA>         <NA>         NA 
      108 <NA>         <NA>         NA 
      109 <NA>         <NA>         NA 
      110 <NA>         <NA>         NA 
      111 <NA>         <NA>         NA 
      112 <NA>         <NA>         NA 
      113 <NA>         <NA>         NA 
      114 <NA>         <NA>         NA 
      115 <NA>         <NA>         NA 
      116 <NA>         <NA>         NA 
      117 <NA>         <NA>         NA 
      118 <NA>         <NA>         NA 
      119 <NA>         <NA>         NA 
      120 <NA>         <NA>         NA 
      121 <NA>         <NA>         NA 
      122 <NA>         <NA>         NA 
      123 <NA>         <NA>         NA 
      124 <NA>         <NA>         NA 
      125 <NA>         <NA>         NA 
      126 <NA>         <NA>         NA 
      127 <NA>         <NA>         NA 
      128 <NA>         <NA>         NA 
      129 <NA>         <NA>         NA 
      130 <NA>         <NA>         NA 
      131 <NA>         <NA>         NA 
      132 <NA>         <NA>         NA 
      133 <NA>         <NA>         NA 
      134 <NA>         <NA>         NA 
      135 <NA>         <NA>         NA 
      136 <NA>         <NA>         NA 
      137 <NA>         <NA>         NA 
      138 <NA>         <NA>         NA 
      139 <NA>         <NA>         NA 
      140 <NA>         <NA>         NA 
      141 <NA>         <NA>         NA 
      142 <NA>         <NA>         NA 
      143 <NA>         <NA>         NA 
      144 <NA>         <NA>         NA 
      145 <NA>         <NA>         NA 
      146 <NA>         <NA>         NA 
      147 <NA>         <NA>         NA 
      148 <NA>         <NA>         NA 
      149 <NA>         <NA>         NA 
      150 <NA>         <NA>         NA 
      151 <NA>         <NA>         NA 
      152 <NA>         <NA>         NA 
      153 <NA>         <NA>         NA 
      154 <NA>         <NA>         NA 
      155 <NA>         <NA>         NA 
      156 <NA>         <NA>         NA 
      157 <NA>         <NA>         NA 
      158 <NA>         <NA>         NA 
      159 <NA>         <NA>         NA 
      160 <NA>         <NA>         NA 
      161 <NA>         <NA>         NA 
      162 <NA>         <NA>         NA 
      163 <NA>         <NA>         NA 
      164 <NA>         <NA>         NA 
      165 <NA>         <NA>         NA 
      166 <NA>         <NA>         NA 
      167 <NA>         <NA>         NA 
      168 <NA>         <NA>         NA 
      169 <NA>         <NA>         NA 
      170 <NA>         <NA>         NA 
      171 <NA>         <NA>         NA 
      172 <NA>         <NA>         NA 
      173 <NA>         <NA>         NA 
      174 <NA>         <NA>         NA 
      175 <NA>         <NA>         NA 
      176 <NA>         <NA>         NA 
      177 <NA>         <NA>         NA 
      178 <NA>         <NA>         NA 
      179 <NA>         <NA>         NA 
      180 <NA>         <NA>         NA 
      181 <NA>         <NA>         NA 
      182 <NA>         <NA>         NA 
      183 <NA>         <NA>         NA 
      184 <NA>         <NA>         NA 
      185 <NA>         <NA>         NA 
      186 <NA>         <NA>         NA 
      187 <NA>         <NA>         NA 
      188 <NA>         <NA>         NA 
      189 <NA>         <NA>         NA 
      190 <NA>         <NA>         NA 
      191 <NA>         <NA>         NA 
      192 <NA>         <NA>         NA 
      193 <NA>         <NA>         NA 
      194 <NA>         <NA>         NA 
      195 <NA>         <NA>         NA 
      196 <NA>         <NA>         NA 
      197 <NA>         <NA>         NA 
      198 <NA>         <NA>         NA 
      199 <NA>         <NA>         NA 
      200 <NA>         <NA>         NA 
      201 <NA>         <NA>         NA 
      202 <NA>         <NA>         NA 
      203 <NA>         <NA>         NA 
      204 <NA>         <NA>         NA 
      205 <NA>         <NA>         NA 
      206 <NA>         <NA>         NA 
      207 <NA>         <NA>         NA 
      208 <NA>         <NA>         NA 
      209 <NA>         <NA>         NA 
      210 <NA>         <NA>         NA 
      211 <NA>         <NA>         NA 
      212 <NA>         <NA>         NA 
      213 <NA>         <NA>         NA 
      214 <NA>         <NA>         NA 
      215 <NA>         <NA>         NA 
      216 <NA>         <NA>         NA 
      217 <NA>         <NA>         NA 
      218 <NA>         <NA>         NA 
      219 <NA>         <NA>         NA 
      220 <NA>         <NA>         NA 
      221 <NA>         <NA>         NA 
      222 <NA>         <NA>         NA 
      223 <NA>         <NA>         NA 
      224 <NA>         <NA>         NA 
      225 <NA>         <NA>         NA 
      226 <NA>         <NA>         NA 
      227 <NA>         <NA>         NA 
      228 <NA>         <NA>         NA 
      229 <NA>         <NA>         NA 
      230 <NA>         <NA>         NA 
      231 <NA>         <NA>         NA 
      232 <NA>         <NA>         NA 
      233 <NA>         <NA>         NA 
      234 <NA>         <NA>         NA 
      235 <NA>         <NA>         NA 
      236 <NA>         <NA>         NA 
      237 <NA>         <NA>         NA 
      238 <NA>         <NA>         NA 
      239 <NA>         <NA>         NA 
      240 <NA>         <NA>         NA 
      241 <NA>         <NA>         NA 
      242 <NA>         <NA>         NA 
      243 <NA>         <NA>         NA 
      244 <NA>         <NA>         NA 
      245 <NA>         <NA>         NA 
      246 <NA>         <NA>         NA 
      247 <NA>         <NA>         NA 
      248 <NA>         <NA>         NA 
      249 <NA>         <NA>         NA 
      250 <NA>         <NA>         NA 
      251 <NA>         <NA>         NA 
      252 <NA>         <NA>         NA 
      253 <NA>         <NA>         NA 
      254 <NA>         <NA>         NA 
      255 <NA>         <NA>         NA 
      256 <NA>         <NA>         NA 
      257 <NA>         <NA>         NA 
      258 <NA>         <NA>         NA 
      259 <NA>         <NA>         NA 
      260 <NA>         <NA>         NA 
      261 <NA>         <NA>         NA 
      262 <NA>         <NA>         NA 
      263 <NA>         <NA>         NA 
      264 <NA>         <NA>         NA 
      265 <NA>         <NA>         NA 
      266 <NA>         <NA>         NA 
      267 <NA>         <NA>         NA 
      268 <NA>         <NA>         NA 
      269 <NA>         <NA>         NA 
      270 <NA>         <NA>         NA 
      271 <NA>         <NA>         NA 
      272 <NA>         <NA>         NA 
      273 <NA>         <NA>         NA 
      274 <NA>         <NA>         NA 
      275 <NA>         <NA>         NA 
      276 <NA>         <NA>         NA 
      277 <NA>         <NA>         NA 
      278 <NA>         <NA>         NA 
      279 <NA>         <NA>         NA 
      280 <NA>         <NA>         NA 
      281 <NA>         <NA>         NA 
      282 <NA>         <NA>         NA 
      283 <NA>         <NA>         NA 
      284 <NA>         <NA>         NA 
      285 <NA>         <NA>         NA 
      286 <NA>         <NA>         NA 
      287 <NA>         <NA>         NA 
      288 <NA>         <NA>         NA 
      289 <NA>         <NA>         NA 
      290 <NA>         <NA>         NA 
      291 <NA>         <NA>         NA 
      292 <NA>         <NA>         NA 
      293 <NA>         <NA>         NA 
      294 <NA>         <NA>         NA 
      295 <NA>         <NA>         NA 
      296 <NA>         <NA>         NA 
      297 <NA>         <NA>         NA 
      298 <NA>         <NA>         NA 
      299 <NA>         <NA>         NA 
      300 <NA>         <NA>         NA 
      301 <NA>         <NA>         NA 
      302 <NA>         <NA>         NA 
      303 <NA>         <NA>         NA 
      304 <NA>         <NA>         NA 
      305 <NA>         <NA>         NA 
      306 <NA>         <NA>         NA 
      307 <NA>         <NA>         NA 
      308 multinomial  data       -836.
      309 multinomial  data       -836.
      310 multinomial  data       -836.
      311 multinomial  data       -836.
      312 multinomial  data       -836.
      313 multinomial  data       -836.
      314 multinomial  data       -836.
      315 multinomial  data       -836.
      316 multinomial  data       -836.
      317 multinomial  data       -836.
      318 multinomial  data       -836.
      319 multinomial  data       -836.
      320 multinomial  data       -836.
      # i 13,927 more rows

# `get_estimates()` works with estimation run

    Code
      print(dplyr::select(get_estimates(readRDS(fit_files[[1]])), -estimated,
      -expected, -uncertainty, -gradient, -likelihood, -log_like_cv, -gradient), n = 320,
      width = Inf)
    Output
      # A tibble: 14,247 x 17
          module_name module_id module_type label                    type   type_id
          <chr>           <int> <chr>       <chr>                    <chr>    <int>
        1 Fleet               1 <NA>        log_Fmort                vector       4
        2 Fleet               1 <NA>        log_Fmort                vector       4
        3 Fleet               1 <NA>        log_Fmort                vector       4
        4 Fleet               1 <NA>        log_Fmort                vector       4
        5 Fleet               1 <NA>        log_Fmort                vector       4
        6 Fleet               1 <NA>        log_Fmort                vector       4
        7 Fleet               1 <NA>        log_Fmort                vector       4
        8 Fleet               1 <NA>        log_Fmort                vector       4
        9 Fleet               1 <NA>        log_Fmort                vector       4
       10 Fleet               1 <NA>        log_Fmort                vector       4
       11 Fleet               1 <NA>        log_Fmort                vector       4
       12 Fleet               1 <NA>        log_Fmort                vector       4
       13 Fleet               1 <NA>        log_Fmort                vector       4
       14 Fleet               1 <NA>        log_Fmort                vector       4
       15 Fleet               1 <NA>        log_Fmort                vector       4
       16 Fleet               1 <NA>        log_Fmort                vector       4
       17 Fleet               1 <NA>        log_Fmort                vector       4
       18 Fleet               1 <NA>        log_Fmort                vector       4
       19 Fleet               1 <NA>        log_Fmort                vector       4
       20 Fleet               1 <NA>        log_Fmort                vector       4
       21 Fleet               1 <NA>        log_Fmort                vector       4
       22 Fleet               1 <NA>        log_Fmort                vector       4
       23 Fleet               1 <NA>        log_Fmort                vector       4
       24 Fleet               1 <NA>        log_Fmort                vector       4
       25 Fleet               1 <NA>        log_Fmort                vector       4
       26 Fleet               1 <NA>        log_Fmort                vector       4
       27 Fleet               1 <NA>        log_Fmort                vector       4
       28 Fleet               1 <NA>        log_Fmort                vector       4
       29 Fleet               1 <NA>        log_Fmort                vector       4
       30 Fleet               1 <NA>        log_Fmort                vector       4
       31 Fleet               1 <NA>        log_q                    vector       3
       32 Fleet               1 <NA>        age_to_length_conversion vector      11
       33 Fleet               1 <NA>        age_to_length_conversion vector      11
       34 Fleet               1 <NA>        age_to_length_conversion vector      11
       35 Fleet               1 <NA>        age_to_length_conversion vector      11
       36 Fleet               1 <NA>        age_to_length_conversion vector      11
       37 Fleet               1 <NA>        age_to_length_conversion vector      11
       38 Fleet               1 <NA>        age_to_length_conversion vector      11
       39 Fleet               1 <NA>        age_to_length_conversion vector      11
       40 Fleet               1 <NA>        age_to_length_conversion vector      11
       41 Fleet               1 <NA>        age_to_length_conversion vector      11
       42 Fleet               1 <NA>        age_to_length_conversion vector      11
       43 Fleet               1 <NA>        age_to_length_conversion vector      11
       44 Fleet               1 <NA>        age_to_length_conversion vector      11
       45 Fleet               1 <NA>        age_to_length_conversion vector      11
       46 Fleet               1 <NA>        age_to_length_conversion vector      11
       47 Fleet               1 <NA>        age_to_length_conversion vector      11
       48 Fleet               1 <NA>        age_to_length_conversion vector      11
       49 Fleet               1 <NA>        age_to_length_conversion vector      11
       50 Fleet               1 <NA>        age_to_length_conversion vector      11
       51 Fleet               1 <NA>        age_to_length_conversion vector      11
       52 Fleet               1 <NA>        age_to_length_conversion vector      11
       53 Fleet               1 <NA>        age_to_length_conversion vector      11
       54 Fleet               1 <NA>        age_to_length_conversion vector      11
       55 Fleet               1 <NA>        age_to_length_conversion vector      11
       56 Fleet               1 <NA>        age_to_length_conversion vector      11
       57 Fleet               1 <NA>        age_to_length_conversion vector      11
       58 Fleet               1 <NA>        age_to_length_conversion vector      11
       59 Fleet               1 <NA>        age_to_length_conversion vector      11
       60 Fleet               1 <NA>        age_to_length_conversion vector      11
       61 Fleet               1 <NA>        age_to_length_conversion vector      11
       62 Fleet               1 <NA>        age_to_length_conversion vector      11
       63 Fleet               1 <NA>        age_to_length_conversion vector      11
       64 Fleet               1 <NA>        age_to_length_conversion vector      11
       65 Fleet               1 <NA>        age_to_length_conversion vector      11
       66 Fleet               1 <NA>        age_to_length_conversion vector      11
       67 Fleet               1 <NA>        age_to_length_conversion vector      11
       68 Fleet               1 <NA>        age_to_length_conversion vector      11
       69 Fleet               1 <NA>        age_to_length_conversion vector      11
       70 Fleet               1 <NA>        age_to_length_conversion vector      11
       71 Fleet               1 <NA>        age_to_length_conversion vector      11
       72 Fleet               1 <NA>        age_to_length_conversion vector      11
       73 Fleet               1 <NA>        age_to_length_conversion vector      11
       74 Fleet               1 <NA>        age_to_length_conversion vector      11
       75 Fleet               1 <NA>        age_to_length_conversion vector      11
       76 Fleet               1 <NA>        age_to_length_conversion vector      11
       77 Fleet               1 <NA>        age_to_length_conversion vector      11
       78 Fleet               1 <NA>        age_to_length_conversion vector      11
       79 Fleet               1 <NA>        age_to_length_conversion vector      11
       80 Fleet               1 <NA>        age_to_length_conversion vector      11
       81 Fleet               1 <NA>        age_to_length_conversion vector      11
       82 Fleet               1 <NA>        age_to_length_conversion vector      11
       83 Fleet               1 <NA>        age_to_length_conversion vector      11
       84 Fleet               1 <NA>        age_to_length_conversion vector      11
       85 Fleet               1 <NA>        age_to_length_conversion vector      11
       86 Fleet               1 <NA>        age_to_length_conversion vector      11
       87 Fleet               1 <NA>        age_to_length_conversion vector      11
       88 Fleet               1 <NA>        age_to_length_conversion vector      11
       89 Fleet               1 <NA>        age_to_length_conversion vector      11
       90 Fleet               1 <NA>        age_to_length_conversion vector      11
       91 Fleet               1 <NA>        age_to_length_conversion vector      11
       92 Fleet               1 <NA>        age_to_length_conversion vector      11
       93 Fleet               1 <NA>        age_to_length_conversion vector      11
       94 Fleet               1 <NA>        age_to_length_conversion vector      11
       95 Fleet               1 <NA>        age_to_length_conversion vector      11
       96 Fleet               1 <NA>        age_to_length_conversion vector      11
       97 Fleet               1 <NA>        age_to_length_conversion vector      11
       98 Fleet               1 <NA>        age_to_length_conversion vector      11
       99 Fleet               1 <NA>        age_to_length_conversion vector      11
      100 Fleet               1 <NA>        age_to_length_conversion vector      11
      101 Fleet               1 <NA>        age_to_length_conversion vector      11
      102 Fleet               1 <NA>        age_to_length_conversion vector      11
      103 Fleet               1 <NA>        age_to_length_conversion vector      11
      104 Fleet               1 <NA>        age_to_length_conversion vector      11
      105 Fleet               1 <NA>        age_to_length_conversion vector      11
      106 Fleet               1 <NA>        age_to_length_conversion vector      11
      107 Fleet               1 <NA>        age_to_length_conversion vector      11
      108 Fleet               1 <NA>        age_to_length_conversion vector      11
      109 Fleet               1 <NA>        age_to_length_conversion vector      11
      110 Fleet               1 <NA>        age_to_length_conversion vector      11
      111 Fleet               1 <NA>        age_to_length_conversion vector      11
      112 Fleet               1 <NA>        age_to_length_conversion vector      11
      113 Fleet               1 <NA>        age_to_length_conversion vector      11
      114 Fleet               1 <NA>        age_to_length_conversion vector      11
      115 Fleet               1 <NA>        age_to_length_conversion vector      11
      116 Fleet               1 <NA>        age_to_length_conversion vector      11
      117 Fleet               1 <NA>        age_to_length_conversion vector      11
      118 Fleet               1 <NA>        age_to_length_conversion vector      11
      119 Fleet               1 <NA>        age_to_length_conversion vector      11
      120 Fleet               1 <NA>        age_to_length_conversion vector      11
      121 Fleet               1 <NA>        age_to_length_conversion vector      11
      122 Fleet               1 <NA>        age_to_length_conversion vector      11
      123 Fleet               1 <NA>        age_to_length_conversion vector      11
      124 Fleet               1 <NA>        age_to_length_conversion vector      11
      125 Fleet               1 <NA>        age_to_length_conversion vector      11
      126 Fleet               1 <NA>        age_to_length_conversion vector      11
      127 Fleet               1 <NA>        age_to_length_conversion vector      11
      128 Fleet               1 <NA>        age_to_length_conversion vector      11
      129 Fleet               1 <NA>        age_to_length_conversion vector      11
      130 Fleet               1 <NA>        age_to_length_conversion vector      11
      131 Fleet               1 <NA>        age_to_length_conversion vector      11
      132 Fleet               1 <NA>        age_to_length_conversion vector      11
      133 Fleet               1 <NA>        age_to_length_conversion vector      11
      134 Fleet               1 <NA>        age_to_length_conversion vector      11
      135 Fleet               1 <NA>        age_to_length_conversion vector      11
      136 Fleet               1 <NA>        age_to_length_conversion vector      11
      137 Fleet               1 <NA>        age_to_length_conversion vector      11
      138 Fleet               1 <NA>        age_to_length_conversion vector      11
      139 Fleet               1 <NA>        age_to_length_conversion vector      11
      140 Fleet               1 <NA>        age_to_length_conversion vector      11
      141 Fleet               1 <NA>        age_to_length_conversion vector      11
      142 Fleet               1 <NA>        age_to_length_conversion vector      11
      143 Fleet               1 <NA>        age_to_length_conversion vector      11
      144 Fleet               1 <NA>        age_to_length_conversion vector      11
      145 Fleet               1 <NA>        age_to_length_conversion vector      11
      146 Fleet               1 <NA>        age_to_length_conversion vector      11
      147 Fleet               1 <NA>        age_to_length_conversion vector      11
      148 Fleet               1 <NA>        age_to_length_conversion vector      11
      149 Fleet               1 <NA>        age_to_length_conversion vector      11
      150 Fleet               1 <NA>        age_to_length_conversion vector      11
      151 Fleet               1 <NA>        age_to_length_conversion vector      11
      152 Fleet               1 <NA>        age_to_length_conversion vector      11
      153 Fleet               1 <NA>        age_to_length_conversion vector      11
      154 Fleet               1 <NA>        age_to_length_conversion vector      11
      155 Fleet               1 <NA>        age_to_length_conversion vector      11
      156 Fleet               1 <NA>        age_to_length_conversion vector      11
      157 Fleet               1 <NA>        age_to_length_conversion vector      11
      158 Fleet               1 <NA>        age_to_length_conversion vector      11
      159 Fleet               1 <NA>        age_to_length_conversion vector      11
      160 Fleet               1 <NA>        age_to_length_conversion vector      11
      161 Fleet               1 <NA>        age_to_length_conversion vector      11
      162 Fleet               1 <NA>        age_to_length_conversion vector      11
      163 Fleet               1 <NA>        age_to_length_conversion vector      11
      164 Fleet               1 <NA>        age_to_length_conversion vector      11
      165 Fleet               1 <NA>        age_to_length_conversion vector      11
      166 Fleet               1 <NA>        age_to_length_conversion vector      11
      167 Fleet               1 <NA>        age_to_length_conversion vector      11
      168 Fleet               1 <NA>        age_to_length_conversion vector      11
      169 Fleet               1 <NA>        age_to_length_conversion vector      11
      170 Fleet               1 <NA>        age_to_length_conversion vector      11
      171 Fleet               1 <NA>        age_to_length_conversion vector      11
      172 Fleet               1 <NA>        age_to_length_conversion vector      11
      173 Fleet               1 <NA>        age_to_length_conversion vector      11
      174 Fleet               1 <NA>        age_to_length_conversion vector      11
      175 Fleet               1 <NA>        age_to_length_conversion vector      11
      176 Fleet               1 <NA>        age_to_length_conversion vector      11
      177 Fleet               1 <NA>        age_to_length_conversion vector      11
      178 Fleet               1 <NA>        age_to_length_conversion vector      11
      179 Fleet               1 <NA>        age_to_length_conversion vector      11
      180 Fleet               1 <NA>        age_to_length_conversion vector      11
      181 Fleet               1 <NA>        age_to_length_conversion vector      11
      182 Fleet               1 <NA>        age_to_length_conversion vector      11
      183 Fleet               1 <NA>        age_to_length_conversion vector      11
      184 Fleet               1 <NA>        age_to_length_conversion vector      11
      185 Fleet               1 <NA>        age_to_length_conversion vector      11
      186 Fleet               1 <NA>        age_to_length_conversion vector      11
      187 Fleet               1 <NA>        age_to_length_conversion vector      11
      188 Fleet               1 <NA>        age_to_length_conversion vector      11
      189 Fleet               1 <NA>        age_to_length_conversion vector      11
      190 Fleet               1 <NA>        age_to_length_conversion vector      11
      191 Fleet               1 <NA>        age_to_length_conversion vector      11
      192 Fleet               1 <NA>        age_to_length_conversion vector      11
      193 Fleet               1 <NA>        age_to_length_conversion vector      11
      194 Fleet               1 <NA>        age_to_length_conversion vector      11
      195 Fleet               1 <NA>        age_to_length_conversion vector      11
      196 Fleet               1 <NA>        age_to_length_conversion vector      11
      197 Fleet               1 <NA>        age_to_length_conversion vector      11
      198 Fleet               1 <NA>        age_to_length_conversion vector      11
      199 Fleet               1 <NA>        age_to_length_conversion vector      11
      200 Fleet               1 <NA>        age_to_length_conversion vector      11
      201 Fleet               1 <NA>        age_to_length_conversion vector      11
      202 Fleet               1 <NA>        age_to_length_conversion vector      11
      203 Fleet               1 <NA>        age_to_length_conversion vector      11
      204 Fleet               1 <NA>        age_to_length_conversion vector      11
      205 Fleet               1 <NA>        age_to_length_conversion vector      11
      206 Fleet               1 <NA>        age_to_length_conversion vector      11
      207 Fleet               1 <NA>        age_to_length_conversion vector      11
      208 Fleet               1 <NA>        age_to_length_conversion vector      11
      209 Fleet               1 <NA>        age_to_length_conversion vector      11
      210 Fleet               1 <NA>        age_to_length_conversion vector      11
      211 Fleet               1 <NA>        age_to_length_conversion vector      11
      212 Fleet               1 <NA>        age_to_length_conversion vector      11
      213 Fleet               1 <NA>        age_to_length_conversion vector      11
      214 Fleet               1 <NA>        age_to_length_conversion vector      11
      215 Fleet               1 <NA>        age_to_length_conversion vector      11
      216 Fleet               1 <NA>        age_to_length_conversion vector      11
      217 Fleet               1 <NA>        age_to_length_conversion vector      11
      218 Fleet               1 <NA>        age_to_length_conversion vector      11
      219 Fleet               1 <NA>        age_to_length_conversion vector      11
      220 Fleet               1 <NA>        age_to_length_conversion vector      11
      221 Fleet               1 <NA>        age_to_length_conversion vector      11
      222 Fleet               1 <NA>        age_to_length_conversion vector      11
      223 Fleet               1 <NA>        age_to_length_conversion vector      11
      224 Fleet               1 <NA>        age_to_length_conversion vector      11
      225 Fleet               1 <NA>        age_to_length_conversion vector      11
      226 Fleet               1 <NA>        age_to_length_conversion vector      11
      227 Fleet               1 <NA>        age_to_length_conversion vector      11
      228 Fleet               1 <NA>        age_to_length_conversion vector      11
      229 Fleet               1 <NA>        age_to_length_conversion vector      11
      230 Fleet               1 <NA>        age_to_length_conversion vector      11
      231 Fleet               1 <NA>        age_to_length_conversion vector      11
      232 Fleet               1 <NA>        age_to_length_conversion vector      11
      233 Fleet               1 <NA>        age_to_length_conversion vector      11
      234 Fleet               1 <NA>        age_to_length_conversion vector      11
      235 Fleet               1 <NA>        age_to_length_conversion vector      11
      236 Fleet               1 <NA>        age_to_length_conversion vector      11
      237 Fleet               1 <NA>        age_to_length_conversion vector      11
      238 Fleet               1 <NA>        age_to_length_conversion vector      11
      239 Fleet               1 <NA>        age_to_length_conversion vector      11
      240 Fleet               1 <NA>        age_to_length_conversion vector      11
      241 Fleet               1 <NA>        age_to_length_conversion vector      11
      242 Fleet               1 <NA>        age_to_length_conversion vector      11
      243 Fleet               1 <NA>        age_to_length_conversion vector      11
      244 Fleet               1 <NA>        age_to_length_conversion vector      11
      245 Fleet               1 <NA>        age_to_length_conversion vector      11
      246 Fleet               1 <NA>        age_to_length_conversion vector      11
      247 Fleet               1 <NA>        age_to_length_conversion vector      11
      248 Fleet               1 <NA>        age_to_length_conversion vector      11
      249 Fleet               1 <NA>        age_to_length_conversion vector      11
      250 Fleet               1 <NA>        age_to_length_conversion vector      11
      251 Fleet               1 <NA>        age_to_length_conversion vector      11
      252 Fleet               1 <NA>        age_to_length_conversion vector      11
      253 Fleet               1 <NA>        age_to_length_conversion vector      11
      254 Fleet               1 <NA>        age_to_length_conversion vector      11
      255 Fleet               1 <NA>        age_to_length_conversion vector      11
      256 Fleet               1 <NA>        age_to_length_conversion vector      11
      257 Fleet               1 <NA>        age_to_length_conversion vector      11
      258 Fleet               1 <NA>        age_to_length_conversion vector      11
      259 Fleet               1 <NA>        age_to_length_conversion vector      11
      260 Fleet               1 <NA>        age_to_length_conversion vector      11
      261 Fleet               1 <NA>        age_to_length_conversion vector      11
      262 Fleet               1 <NA>        age_to_length_conversion vector      11
      263 Fleet               1 <NA>        age_to_length_conversion vector      11
      264 Fleet               1 <NA>        age_to_length_conversion vector      11
      265 Fleet               1 <NA>        age_to_length_conversion vector      11
      266 Fleet               1 <NA>        age_to_length_conversion vector      11
      267 Fleet               1 <NA>        age_to_length_conversion vector      11
      268 Fleet               1 <NA>        age_to_length_conversion vector      11
      269 Fleet               1 <NA>        age_to_length_conversion vector      11
      270 Fleet               1 <NA>        age_to_length_conversion vector      11
      271 Fleet               1 <NA>        age_to_length_conversion vector      11
      272 Fleet               1 <NA>        age_to_length_conversion vector      11
      273 Fleet               1 <NA>        age_to_length_conversion vector      11
      274 Fleet               1 <NA>        age_to_length_conversion vector      11
      275 Fleet               1 <NA>        age_to_length_conversion vector      11
      276 Fleet               1 <NA>        age_to_length_conversion vector      11
      277 Fleet               1 <NA>        age_to_length_conversion vector      11
      278 Fleet               1 <NA>        age_to_length_conversion vector      11
      279 Fleet               1 <NA>        age_to_length_conversion vector      11
      280 Fleet               1 <NA>        age_to_length_conversion vector      11
      281 Fleet               1 <NA>        age_to_length_conversion vector      11
      282 Fleet               1 <NA>        age_to_length_conversion vector      11
      283 Fleet               1 <NA>        age_to_length_conversion vector      11
      284 Fleet               1 <NA>        age_to_length_conversion vector      11
      285 Fleet               1 <NA>        age_to_length_conversion vector      11
      286 Fleet               1 <NA>        age_to_length_conversion vector      11
      287 Fleet               1 <NA>        age_to_length_conversion vector      11
      288 Fleet               1 <NA>        age_to_length_conversion vector      11
      289 Fleet               1 <NA>        age_to_length_conversion vector      11
      290 Fleet               1 <NA>        age_to_length_conversion vector      11
      291 Fleet               1 <NA>        age_to_length_conversion vector      11
      292 Fleet               1 <NA>        age_to_length_conversion vector      11
      293 Fleet               1 <NA>        age_to_length_conversion vector      11
      294 Fleet               1 <NA>        age_to_length_conversion vector      11
      295 Fleet               1 <NA>        age_to_length_conversion vector      11
      296 Fleet               1 <NA>        age_to_length_conversion vector      11
      297 Fleet               1 <NA>        age_to_length_conversion vector      11
      298 Fleet               1 <NA>        age_to_length_conversion vector      11
      299 Fleet               1 <NA>        age_to_length_conversion vector      11
      300 Fleet               1 <NA>        age_to_length_conversion vector      11
      301 Fleet               1 <NA>        age_to_length_conversion vector      11
      302 Fleet               1 <NA>        age_to_length_conversion vector      11
      303 Fleet               1 <NA>        age_to_length_conversion vector      11
      304 Fleet               1 <NA>        age_to_length_conversion vector      11
      305 Fleet               1 <NA>        age_to_length_conversion vector      11
      306 Fleet               1 <NA>        age_to_length_conversion vector      11
      307 Fleet               1 <NA>        age_to_length_conversion vector      11
      308 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      309 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      310 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      311 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      312 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      313 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      314 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      315 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      316 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      317 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      318 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      319 Fleet               1 <NA>        agecomp_expected         <NA>        NA
      320 Fleet               1 <NA>        agecomp_expected         <NA>        NA
          parameter_id fleet year_i age_i length_i     input observed estimation_type 
                 <int> <chr>  <int> <int>    <int>     <dbl>    <dbl> <chr>           
        1            4 NA         1    NA       NA -4.66e+ 0       NA fixed_effects   
        2          287 NA         2    NA       NA -3.60e+ 0       NA fixed_effects   
        3          288 NA         3    NA       NA -3.10e+ 0       NA fixed_effects   
        4          289 NA         4    NA       NA -2.80e+ 0       NA fixed_effects   
        5          290 NA         5    NA       NA -3.02e+ 0       NA fixed_effects   
        6          291 NA         6    NA       NA -2.44e+ 0       NA fixed_effects   
        7          292 NA         7    NA       NA -2.43e+ 0       NA fixed_effects   
        8          293 NA         8    NA       NA -1.68e+ 0       NA fixed_effects   
        9          294 NA         9    NA       NA -2.22e+ 0       NA fixed_effects   
       10          295 NA        10    NA       NA -2.02e+ 0       NA fixed_effects   
       11          296 NA        11    NA       NA -1.89e+ 0       NA fixed_effects   
       12          297 NA        12    NA       NA -1.82e+ 0       NA fixed_effects   
       13          298 NA        13    NA       NA -2.15e+ 0       NA fixed_effects   
       14          299 NA        14    NA       NA -1.78e+ 0       NA fixed_effects   
       15          300 NA        15    NA       NA -1.71e+ 0       NA fixed_effects   
       16          301 NA        16    NA       NA -1.82e+ 0       NA fixed_effects   
       17          302 NA        17    NA       NA -1.16e+ 0       NA fixed_effects   
       18          303 NA        18    NA       NA -1.36e+ 0       NA fixed_effects   
       19          304 NA        19    NA       NA -1.37e+ 0       NA fixed_effects   
       20          305 NA        20    NA       NA -1.38e+ 0       NA fixed_effects   
       21          306 NA        21    NA       NA -1.05e+ 0       NA fixed_effects   
       22          307 NA        22    NA       NA -1.37e+ 0       NA fixed_effects   
       23          308 NA        23    NA       NA -8.71e- 1       NA fixed_effects   
       24          309 NA        24    NA       NA -1.06e+ 0       NA fixed_effects   
       25          310 NA        25    NA       NA -1.07e+ 0       NA fixed_effects   
       26          311 NA        26    NA       NA -1.16e+ 0       NA fixed_effects   
       27          312 NA        27    NA       NA -1.18e+ 0       NA fixed_effects   
       28          313 NA        28    NA       NA -8.40e- 1       NA fixed_effects   
       29          314 NA        29    NA       NA -1.11e+ 0       NA fixed_effects   
       30          315 NA        30    NA       NA -6.94e- 1       NA fixed_effects   
       31            3 NA        NA    NA       NA  0              NA constant        
       32           11 NA        NA     1        1  1.26e-16       NA constant        
       33           12 NA        NA     1        2  8.39e-11       NA constant        
       34           13 NA        NA     1        3  2.30e- 6       NA constant        
       35           14 NA        NA     1        4  2.74e- 3       NA constant        
       36           15 NA        NA     1        5  1.63e- 1       NA constant        
       37           16 NA        NA     1        6  6.32e- 1       NA constant        
       38           17 NA        NA     1        7  1.98e- 1       NA constant        
       39           18 NA        NA     1        8  4.13e- 3       NA constant        
       40           19 NA        NA     1        9  4.36e- 6       NA constant        
       41           20 NA        NA     1       10  2.02e-10       NA constant        
       42           21 NA        NA     1       11  3.33e-16       NA constant        
       43           22 NA        NA     1       12  0              NA constant        
       44           23 NA        NA     1       13  0              NA constant        
       45           24 NA        NA     1       14  0              NA constant        
       46           25 NA        NA     1       15  0              NA constant        
       47           26 NA        NA     1       16  0              NA constant        
       48           27 NA        NA     1       17  0              NA constant        
       49           28 NA        NA     1       18  0              NA constant        
       50           29 NA        NA     1       19  0              NA constant        
       51           30 NA        NA     1       20  0              NA constant        
       52           31 NA        NA     1       21  0              NA constant        
       53           32 NA        NA     1       22  0              NA constant        
       54           33 NA        NA     1       23  0              NA constant        
       55           34 NA        NA     2        1  3.27e-18       NA constant        
       56           35 NA        NA     2        2  2.15e-13       NA constant        
       57           36 NA        NA     2        3  2.20e- 9       NA constant        
       58           37 NA        NA     2        4  3.54e- 6       NA constant        
       59           38 NA        NA     2        5  9.19e- 4       NA constant        
       60           39 NA        NA     2        6  4.03e- 2       NA constant        
       61           40 NA        NA     2        7  3.18e- 1       NA constant        
       62           41 NA        NA     2        8  4.86e- 1       NA constant        
       63           42 NA        NA     2        9  1.46e- 1       NA constant        
       64           43 NA        NA     2       10  8.23e- 3       NA constant        
       65           44 NA        NA     2       11  8.08e- 5       NA constant        
       66           45 NA        NA     2       12  1.31e- 7       NA constant        
       67           46 NA        NA     2       13  3.37e-11       NA constant        
       68           47 NA        NA     2       14  1.33e-15       NA constant        
       69           48 NA        NA     2       15  0              NA constant        
       70           49 NA        NA     2       16  0              NA constant        
       71           50 NA        NA     2       17  0              NA constant        
       72           51 NA        NA     2       18  0              NA constant        
       73           52 NA        NA     2       19  0              NA constant        
       74           53 NA        NA     2       20  0              NA constant        
       75           54 NA        NA     2       21  0              NA constant        
       76           55 NA        NA     2       22  0              NA constant        
       77           56 NA        NA     2       23  0              NA constant        
       78           57 NA        NA     3        1  4.35e-19       NA constant        
       79           58 NA        NA     3        2  6.73e-15       NA constant        
       80           59 NA        NA     3        3  2.84e-11       NA constant        
       81           60 NA        NA     3        4  3.28e- 8       NA constant        
       82           61 NA        NA     3        5  1.05e- 5       NA constant        
       83           62 NA        NA     3        6  9.44e- 4       NA constant        
       84           63 NA        NA     3        7  2.44e- 2       NA constant        
       85           64 NA        NA     3        8  1.85e- 1       NA constant        
       86           65 NA        NA     3        9  4.24e- 1       NA constant        
       87           66 NA        NA     3       10  2.98e- 1       NA constant        
       88           67 NA        NA     3       11  6.36e- 2       NA constant        
       89           68 NA        NA     3       12  4.04e- 3       NA constant        
       90           69 NA        NA     3       13  7.43e- 5       NA constant        
       91           70 NA        NA     3       14  3.87e- 7       NA constant        
       92           71 NA        NA     3       15  5.62e-10       NA constant        
       93           72 NA        NA     3       16  2.24e-13       NA constant        
       94           73 NA        NA     3       17  0              NA constant        
       95           74 NA        NA     3       18  0              NA constant        
       96           75 NA        NA     3       19  0              NA constant        
       97           76 NA        NA     3       20  0              NA constant        
       98           77 NA        NA     3       21  0              NA constant        
       99           78 NA        NA     3       22  0              NA constant        
      100           79 NA        NA     3       23  0              NA constant        
      101           80 NA        NA     4        1  1.23e-19       NA constant        
      102           81 NA        NA     4        2  7.29e-16       NA constant        
      103           82 NA        NA     4        3  1.58e-12       NA constant        
      104           83 NA        NA     4        4  1.25e- 9       NA constant        
      105           84 NA        NA     4        5  3.68e- 7       NA constant        
      106           85 NA        NA     4        6  4.02e- 5       NA constant        
      107           86 NA        NA     4        7  1.65e- 3       NA constant        
      108           87 NA        NA     4        8  2.56e- 2       NA constant        
      109           88 NA        NA     4        9  1.54e- 1       NA constant        
      110           89 NA        NA     4       10  3.58e- 1       NA constant        
      111           90 NA        NA     4       11  3.27e- 1       NA constant        
      112           91 NA        NA     4       12  1.17e- 1       NA constant        
      113           92 NA        NA     4       13  1.62e- 2       NA constant        
      114           93 NA        NA     4       14  8.65e- 4       NA constant        
      115           94 NA        NA     4       15  1.75e- 5       NA constant        
      116           95 NA        NA     4       16  1.32e- 7       NA constant        
      117           96 NA        NA     4       17  3.71e-10       NA constant        
      118           97 NA        NA     4       18  3.85e-13       NA constant        
      119           98 NA        NA     4       19  1.11e-16       NA constant        
      120           99 NA        NA     4       20  0              NA constant        
      121          100 NA        NA     4       21  0              NA constant        
      122          101 NA        NA     4       22  0              NA constant        
      123          102 NA        NA     4       23  0              NA constant        
      124          103 NA        NA     5        1  5.27e-20       NA constant        
      125          104 NA        NA     5        2  1.59e-16       NA constant        
      126          105 NA        NA     5        3  2.09e-13       NA constant        
      127          106 NA        NA     5        4  1.20e-10       NA constant        
      128          107 NA        NA     5        5  3.04e- 8       NA constant        
      129          108 NA        NA     5        6  3.38e- 6       NA constant        
      130          109 NA        NA     5        7  1.67e- 4       NA constant        
      131          110 NA        NA     5        8  3.67e- 3       NA constant        
      132          111 NA        NA     5        9  3.63e- 2       NA constant        
      133          112 NA        NA     5       10  1.63e- 1       NA constant        
      134          113 NA        NA     5       11  3.31e- 1       NA constant        
      135          114 NA        NA     5       12  3.08e- 1       NA constant        
      136          115 NA        NA     5       13  1.31e- 1       NA constant        
      137          116 NA        NA     5       14  2.52e- 2       NA constant        
      138          117 NA        NA     5       15  2.20e- 3       NA constant        
      139          118 NA        NA     5       16  8.62e- 5       NA constant        
      140          119 NA        NA     5       17  1.50e- 6       NA constant        
      141          120 NA        NA     5       18  1.16e- 8       NA constant        
      142          121 NA        NA     5       19  3.94e-11       NA constant        
      143          122 NA        NA     5       20  5.88e-14       NA constant        
      144          123 NA        NA     5       21  0              NA constant        
      145          124 NA        NA     5       22  0              NA constant        
      146          125 NA        NA     5       23  0              NA constant        
      147          126 NA        NA     6        1  2.88e-20       NA constant        
      148          127 NA        NA     6        2  5.32e-17       NA constant        
      149          128 NA        NA     6        3  4.81e-14       NA constant        
      150          129 NA        NA     6        4  2.13e-11       NA constant        
      151          130 NA        NA     6        5  4.62e- 9       NA constant        
      152          131 NA        NA     6        6  4.94e- 7       NA constant        
      153          132 NA        NA     6        7  2.61e- 5       NA constant        
      154          133 NA        NA     6        8  6.86e- 4       NA constant        
      155          134 NA        NA     6        9  8.98e- 3       NA constant        
      156          135 NA        NA     6       10  5.89e- 2       NA constant        
      157          136 NA        NA     6       11  1.94e- 1       NA constant        
      158          137 NA        NA     6       12  3.23e- 1       NA constant        
      159          138 NA        NA     6       13  2.72e- 1       NA constant        
      160          139 NA        NA     6       14  1.15e- 1       NA constant        
      161          140 NA        NA     6       15  2.47e- 2       NA constant        
      162          141 NA        NA     6       16  2.66e- 3       NA constant        
      163          142 NA        NA     6       17  1.43e- 4       NA constant        
      164          143 NA        NA     6       18  3.81e- 6       NA constant        
      165          144 NA        NA     6       19  5.04e- 8       NA constant        
      166          145 NA        NA     6       20  3.29e-10       NA constant        
      167          146 NA        NA     6       21  1.06e-12       NA constant        
      168          147 NA        NA     6       22  1.67e-15       NA constant        
      169          148 NA        NA     6       23  0              NA constant        
      170          149 NA        NA     7        1  1.85e-20       NA constant        
      171          150 NA        NA     7        2  2.36e-17       NA constant        
      172          151 NA        NA     7        3  1.60e-14       NA constant        
      173          152 NA        NA     7        4  5.73e-12       NA constant        
      174          153 NA        NA     7        5  1.09e- 9       NA constant        
      175          154 NA        NA     7        6  1.10e- 7       NA constant        
      176          155 NA        NA     7        7  5.93e- 6       NA constant        
      177          156 NA        NA     7        8  1.71e- 4       NA constant        
      178          157 NA        NA     7        9  2.63e- 3       NA constant        
      179          158 NA        NA     7       10  2.18e- 2       NA constant        
      180          159 NA        NA     7       11  9.79e- 2       NA constant        
      181          160 NA        NA     7       12  2.37e- 1       NA constant        
      182          161 NA        NA     7       13  3.12e- 1       NA constant        
      183          162 NA        NA     7       14  2.22e- 1       NA constant        
      184          163 NA        NA     7       15  8.59e- 2       NA constant        
      185          164 NA        NA     7       16  1.80e- 2       NA constant        
      186          165 NA        NA     7       17  2.03e- 3       NA constant        
      187          166 NA        NA     7       18  1.23e- 4       NA constant        
      188          167 NA        NA     7       19  4.00e- 6       NA constant        
      189          168 NA        NA     7       20  6.96e- 8       NA constant        
      190          169 NA        NA     7       21  6.44e-10       NA constant        
      191          170 NA        NA     7       22  3.17e-12       NA constant        
      192          171 NA        NA     7       23  8.33e-15       NA constant        
      193          172 NA        NA     8        1  1.32e-20       NA constant        
      194          173 NA        NA     8        2  1.28e-17       NA constant        
      195          174 NA        NA     8        3  6.90e-15       NA constant        
      196          175 NA        NA     8        4  2.09e-12       NA constant        
      197          176 NA        NA     8        5  3.54e-10       NA constant        
      198          177 NA        NA     8        6  3.37e- 8       NA constant        
      199          178 NA        NA     8        7  1.81e- 6       NA constant        
      200          179 NA        NA     8        8  5.45e- 5       NA constant        
      201          180 NA        NA     8        9  9.31e- 4       NA constant        
      202          181 NA        NA     8       10  9.00e- 3       NA constant        
      203          182 NA        NA     8       11  4.95e- 2       NA constant        
      204          183 NA        NA     8       12  1.55e- 1       NA constant        
      205          184 NA        NA     8       13  2.76e- 1       NA constant        
      206          185 NA        NA     8       14  2.81e- 1       NA constant        
      207          186 NA        NA     8       15  1.63e- 1       NA constant        
      208          187 NA        NA     8       16  5.42e- 2       NA constant        
      209          188 NA        NA     8       17  1.02e- 2       NA constant        
      210          189 NA        NA     8       18  1.10e- 3       NA constant        
      211          190 NA        NA     8       19  6.66e- 5       NA constant        
      212          191 NA        NA     8       20  2.29e- 6       NA constant        
      213          192 NA        NA     8       21  4.44e- 8       NA constant        
      214          193 NA        NA     8       22  4.83e-10       NA constant        
      215          194 NA        NA     8       23  2.97e-12       NA constant        
      216          195 NA        NA     9        1  1.02e-20       NA constant        
      217          196 NA        NA     9        2  7.91e-18       NA constant        
      218          197 NA        NA     9        3  3.58e-15       NA constant        
      219          198 NA        NA     9        4  9.46e-13       NA constant        
      220          199 NA        NA     9        5  1.46e-10       NA constant        
      221          200 NA        NA     9        6  1.31e- 8       NA constant        
      222          201 NA        NA     9        7  6.94e- 7       NA constant        
      223          202 NA        NA     9        8  2.15e- 5       NA constant        
      224          203 NA        NA     9        9  3.91e- 4       NA constant        
      225          204 NA        NA     9       10  4.19e- 3       NA constant        
      226          205 NA        NA     9       11  2.65e- 2       NA constant        
      227          206 NA        NA     9       12  9.92e- 2       NA constant        
      228          207 NA        NA     9       13  2.20e- 1       NA constant        
      229          208 NA        NA     9       14  2.88e- 1       NA constant        
      230          209 NA        NA     9       15  2.25e- 1       NA constant        
      231          210 NA        NA     9       16  1.04e- 1       NA constant        
      232          211 NA        NA     9       17  2.83e- 2       NA constant        
      233          212 NA        NA     9       18  4.58e- 3       NA constant        
      234          213 NA        NA     9       19  4.37e- 4       NA constant        
      235          214 NA        NA     9       20  2.46e- 5       NA constant        
      236          215 NA        NA     9       21  8.11e- 7       NA constant        
      237          216 NA        NA     9       22  1.57e- 8       NA constant        
      238          217 NA        NA     9       23  1.80e-10       NA constant        
      239          218 NA        NA    10        1  8.31e-21       NA constant        
      240          219 NA        NA    10        2  5.43e-18       NA constant        
      241          220 NA        NA    10        3  2.14e-15       NA constant        
      242          221 NA        NA    10        4  5.06e-13       NA constant        
      243          222 NA        NA    10        5  7.20e-11       NA constant        
      244          223 NA        NA    10        6  6.18e- 9       NA constant        
      245          224 NA        NA    10        7  3.20e- 7       NA constant        
      246          225 NA        NA    10        8  1.00e- 5       NA constant        
      247          226 NA        NA    10        9  1.91e- 4       NA constant        
      248          227 NA        NA    10       10  2.19e- 3       NA constant        
      249          228 NA        NA    10       11  1.53e- 2       NA constant        
      250          229 NA        NA    10       12  6.53e- 2       NA constant        
      251          230 NA        NA    10       13  1.69e- 1       NA constant        
      252          231 NA        NA    10       14  2.68e- 1       NA constant        
      253          232 NA        NA    10       15  2.59e- 1       NA constant        
      254          233 NA        NA    10       16  1.53e- 1       NA constant        
      255          234 NA        NA    10       17  5.48e- 2       NA constant        
      256          235 NA        NA    10       18  1.20e- 2       NA constant        
      257          236 NA        NA    10       19  1.60e- 3       NA constant        
      258          237 NA        NA    10       20  1.30e- 4       NA constant        
      259          238 NA        NA    10       21  6.37e- 6       NA constant        
      260          239 NA        NA    10       22  1.89e- 7       NA constant        
      261          240 NA        NA    10       23  3.44e- 9       NA constant        
      262          241 NA        NA    11        1  7.07e-21       NA constant        
      263          242 NA        NA    11        2  4.03e-18       NA constant        
      264          243 NA        NA    11        3  1.42e-15       NA constant        
      265          244 NA        NA    11        4  3.06e-13       NA constant        
      266          245 NA        NA    11        5  4.09e-11       NA constant        
      267          246 NA        NA    11        6  3.36e- 9       NA constant        
      268          247 NA        NA    11        7  1.71e- 7       NA constant        
      269          248 NA        NA    11        8  5.39e- 6       NA constant        
      270          249 NA        NA    11        9  1.05e- 4       NA constant        
      271          250 NA        NA    11       10  1.27e- 3       NA constant        
      272          251 NA        NA    11       11  9.57e- 3       NA constant        
      273          252 NA        NA    11       12  4.48e- 2       NA constant        
      274          253 NA        NA    11       13  1.31e- 1       NA constant        
      275          254 NA        NA    11       14  2.38e- 1       NA constant        
      276          255 NA        NA    11       15  2.71e- 1       NA constant        
      277          256 NA        NA    11       16  1.92e- 1       NA constant        
      278          257 NA        NA    11       17  8.49e- 2       NA constant        
      279          258 NA        NA    11       18  2.34e- 2       NA constant        
      280          259 NA        NA    11       19  4.02e- 3       NA constant        
      281          260 NA        NA    11       20  4.30e- 4       NA constant        
      282          261 NA        NA    11       21  2.85e- 5       NA constant        
      283          262 NA        NA    11       22  1.17e- 6       NA constant        
      284          263 NA        NA    11       23  3.04e- 8       NA constant        
      285          264 NA        NA    12        1  6.22e-21       NA constant        
      286          265 NA        NA    12        2  3.17e-18       NA constant        
      287          266 NA        NA    12        3  1.02e-15       NA constant        
      288          267 NA        NA    12        4  2.04e-13       NA constant        
      289          268 NA        NA    12        5  2.58e-11       NA constant        
      290          269 NA        NA    12        6  2.05e- 9       NA constant        
      291          270 NA        NA    12        7  1.03e- 7       NA constant        
      292          271 NA        NA    12        8  3.23e- 6       NA constant        
      293          272 NA        NA    12        9  6.43e- 5       NA constant        
      294          273 NA        NA    12       10  8.07e- 4       NA constant        
      295          274 NA        NA    12       11  6.41e- 3       NA constant        
      296          275 NA        NA    12       12  3.23e- 2       NA constant        
      297          276 NA        NA    12       13  1.03e- 1       NA constant        
      298          277 NA        NA    12       14  2.09e- 1       NA constant        
      299          278 NA        NA    12       15  2.69e- 1       NA constant        
      300          279 NA        NA    12       16  2.20e- 1       NA constant        
      301          280 NA        NA    12       17  1.14e- 1       NA constant        
      302          281 NA        NA    12       18  3.76e- 2       NA constant        
      303          282 NA        NA    12       19  7.85e- 3       NA constant        
      304          283 NA        NA    12       20  1.04e- 3       NA constant        
      305          284 NA        NA    12       21  8.70e- 5       NA constant        
      306          285 NA        NA    12       22  4.61e- 6       NA constant        
      307          286 NA        NA    12       23  1.57e- 7       NA constant        
      308           NA NA         1     1       NA NA              14 derived_quantity
      309           NA NA         1     2       NA NA              20 derived_quantity
      310           NA NA         1     3       NA NA              23 derived_quantity
      311           NA NA         1     4       NA NA              30 derived_quantity
      312           NA NA         1     5       NA NA              20 derived_quantity
      313           NA NA         1     6       NA NA              10 derived_quantity
      314           NA NA         1     7       NA NA              22 derived_quantity
      315           NA NA         1     8       NA NA              11 derived_quantity
      316           NA NA         1     9       NA NA              12 derived_quantity
      317           NA NA         1    10       NA NA               6 derived_quantity
      318           NA NA         1    11       NA NA               7 derived_quantity
      319           NA NA         1    12       NA NA              25 derived_quantity
      320           NA NA         2     1       NA NA              21 derived_quantity
          distribution input_type  lpdf
          <chr>        <chr>      <dbl>
        1 <NA>         <NA>         NA 
        2 <NA>         <NA>         NA 
        3 <NA>         <NA>         NA 
        4 <NA>         <NA>         NA 
        5 <NA>         <NA>         NA 
        6 <NA>         <NA>         NA 
        7 <NA>         <NA>         NA 
        8 <NA>         <NA>         NA 
        9 <NA>         <NA>         NA 
       10 <NA>         <NA>         NA 
       11 <NA>         <NA>         NA 
       12 <NA>         <NA>         NA 
       13 <NA>         <NA>         NA 
       14 <NA>         <NA>         NA 
       15 <NA>         <NA>         NA 
       16 <NA>         <NA>         NA 
       17 <NA>         <NA>         NA 
       18 <NA>         <NA>         NA 
       19 <NA>         <NA>         NA 
       20 <NA>         <NA>         NA 
       21 <NA>         <NA>         NA 
       22 <NA>         <NA>         NA 
       23 <NA>         <NA>         NA 
       24 <NA>         <NA>         NA 
       25 <NA>         <NA>         NA 
       26 <NA>         <NA>         NA 
       27 <NA>         <NA>         NA 
       28 <NA>         <NA>         NA 
       29 <NA>         <NA>         NA 
       30 <NA>         <NA>         NA 
       31 <NA>         <NA>         NA 
       32 <NA>         <NA>         NA 
       33 <NA>         <NA>         NA 
       34 <NA>         <NA>         NA 
       35 <NA>         <NA>         NA 
       36 <NA>         <NA>         NA 
       37 <NA>         <NA>         NA 
       38 <NA>         <NA>         NA 
       39 <NA>         <NA>         NA 
       40 <NA>         <NA>         NA 
       41 <NA>         <NA>         NA 
       42 <NA>         <NA>         NA 
       43 <NA>         <NA>         NA 
       44 <NA>         <NA>         NA 
       45 <NA>         <NA>         NA 
       46 <NA>         <NA>         NA 
       47 <NA>         <NA>         NA 
       48 <NA>         <NA>         NA 
       49 <NA>         <NA>         NA 
       50 <NA>         <NA>         NA 
       51 <NA>         <NA>         NA 
       52 <NA>         <NA>         NA 
       53 <NA>         <NA>         NA 
       54 <NA>         <NA>         NA 
       55 <NA>         <NA>         NA 
       56 <NA>         <NA>         NA 
       57 <NA>         <NA>         NA 
       58 <NA>         <NA>         NA 
       59 <NA>         <NA>         NA 
       60 <NA>         <NA>         NA 
       61 <NA>         <NA>         NA 
       62 <NA>         <NA>         NA 
       63 <NA>         <NA>         NA 
       64 <NA>         <NA>         NA 
       65 <NA>         <NA>         NA 
       66 <NA>         <NA>         NA 
       67 <NA>         <NA>         NA 
       68 <NA>         <NA>         NA 
       69 <NA>         <NA>         NA 
       70 <NA>         <NA>         NA 
       71 <NA>         <NA>         NA 
       72 <NA>         <NA>         NA 
       73 <NA>         <NA>         NA 
       74 <NA>         <NA>         NA 
       75 <NA>         <NA>         NA 
       76 <NA>         <NA>         NA 
       77 <NA>         <NA>         NA 
       78 <NA>         <NA>         NA 
       79 <NA>         <NA>         NA 
       80 <NA>         <NA>         NA 
       81 <NA>         <NA>         NA 
       82 <NA>         <NA>         NA 
       83 <NA>         <NA>         NA 
       84 <NA>         <NA>         NA 
       85 <NA>         <NA>         NA 
       86 <NA>         <NA>         NA 
       87 <NA>         <NA>         NA 
       88 <NA>         <NA>         NA 
       89 <NA>         <NA>         NA 
       90 <NA>         <NA>         NA 
       91 <NA>         <NA>         NA 
       92 <NA>         <NA>         NA 
       93 <NA>         <NA>         NA 
       94 <NA>         <NA>         NA 
       95 <NA>         <NA>         NA 
       96 <NA>         <NA>         NA 
       97 <NA>         <NA>         NA 
       98 <NA>         <NA>         NA 
       99 <NA>         <NA>         NA 
      100 <NA>         <NA>         NA 
      101 <NA>         <NA>         NA 
      102 <NA>         <NA>         NA 
      103 <NA>         <NA>         NA 
      104 <NA>         <NA>         NA 
      105 <NA>         <NA>         NA 
      106 <NA>         <NA>         NA 
      107 <NA>         <NA>         NA 
      108 <NA>         <NA>         NA 
      109 <NA>         <NA>         NA 
      110 <NA>         <NA>         NA 
      111 <NA>         <NA>         NA 
      112 <NA>         <NA>         NA 
      113 <NA>         <NA>         NA 
      114 <NA>         <NA>         NA 
      115 <NA>         <NA>         NA 
      116 <NA>         <NA>         NA 
      117 <NA>         <NA>         NA 
      118 <NA>         <NA>         NA 
      119 <NA>         <NA>         NA 
      120 <NA>         <NA>         NA 
      121 <NA>         <NA>         NA 
      122 <NA>         <NA>         NA 
      123 <NA>         <NA>         NA 
      124 <NA>         <NA>         NA 
      125 <NA>         <NA>         NA 
      126 <NA>         <NA>         NA 
      127 <NA>         <NA>         NA 
      128 <NA>         <NA>         NA 
      129 <NA>         <NA>         NA 
      130 <NA>         <NA>         NA 
      131 <NA>         <NA>         NA 
      132 <NA>         <NA>         NA 
      133 <NA>         <NA>         NA 
      134 <NA>         <NA>         NA 
      135 <NA>         <NA>         NA 
      136 <NA>         <NA>         NA 
      137 <NA>         <NA>         NA 
      138 <NA>         <NA>         NA 
      139 <NA>         <NA>         NA 
      140 <NA>         <NA>         NA 
      141 <NA>         <NA>         NA 
      142 <NA>         <NA>         NA 
      143 <NA>         <NA>         NA 
      144 <NA>         <NA>         NA 
      145 <NA>         <NA>         NA 
      146 <NA>         <NA>         NA 
      147 <NA>         <NA>         NA 
      148 <NA>         <NA>         NA 
      149 <NA>         <NA>         NA 
      150 <NA>         <NA>         NA 
      151 <NA>         <NA>         NA 
      152 <NA>         <NA>         NA 
      153 <NA>         <NA>         NA 
      154 <NA>         <NA>         NA 
      155 <NA>         <NA>         NA 
      156 <NA>         <NA>         NA 
      157 <NA>         <NA>         NA 
      158 <NA>         <NA>         NA 
      159 <NA>         <NA>         NA 
      160 <NA>         <NA>         NA 
      161 <NA>         <NA>         NA 
      162 <NA>         <NA>         NA 
      163 <NA>         <NA>         NA 
      164 <NA>         <NA>         NA 
      165 <NA>         <NA>         NA 
      166 <NA>         <NA>         NA 
      167 <NA>         <NA>         NA 
      168 <NA>         <NA>         NA 
      169 <NA>         <NA>         NA 
      170 <NA>         <NA>         NA 
      171 <NA>         <NA>         NA 
      172 <NA>         <NA>         NA 
      173 <NA>         <NA>         NA 
      174 <NA>         <NA>         NA 
      175 <NA>         <NA>         NA 
      176 <NA>         <NA>         NA 
      177 <NA>         <NA>         NA 
      178 <NA>         <NA>         NA 
      179 <NA>         <NA>         NA 
      180 <NA>         <NA>         NA 
      181 <NA>         <NA>         NA 
      182 <NA>         <NA>         NA 
      183 <NA>         <NA>         NA 
      184 <NA>         <NA>         NA 
      185 <NA>         <NA>         NA 
      186 <NA>         <NA>         NA 
      187 <NA>         <NA>         NA 
      188 <NA>         <NA>         NA 
      189 <NA>         <NA>         NA 
      190 <NA>         <NA>         NA 
      191 <NA>         <NA>         NA 
      192 <NA>         <NA>         NA 
      193 <NA>         <NA>         NA 
      194 <NA>         <NA>         NA 
      195 <NA>         <NA>         NA 
      196 <NA>         <NA>         NA 
      197 <NA>         <NA>         NA 
      198 <NA>         <NA>         NA 
      199 <NA>         <NA>         NA 
      200 <NA>         <NA>         NA 
      201 <NA>         <NA>         NA 
      202 <NA>         <NA>         NA 
      203 <NA>         <NA>         NA 
      204 <NA>         <NA>         NA 
      205 <NA>         <NA>         NA 
      206 <NA>         <NA>         NA 
      207 <NA>         <NA>         NA 
      208 <NA>         <NA>         NA 
      209 <NA>         <NA>         NA 
      210 <NA>         <NA>         NA 
      211 <NA>         <NA>         NA 
      212 <NA>         <NA>         NA 
      213 <NA>         <NA>         NA 
      214 <NA>         <NA>         NA 
      215 <NA>         <NA>         NA 
      216 <NA>         <NA>         NA 
      217 <NA>         <NA>         NA 
      218 <NA>         <NA>         NA 
      219 <NA>         <NA>         NA 
      220 <NA>         <NA>         NA 
      221 <NA>         <NA>         NA 
      222 <NA>         <NA>         NA 
      223 <NA>         <NA>         NA 
      224 <NA>         <NA>         NA 
      225 <NA>         <NA>         NA 
      226 <NA>         <NA>         NA 
      227 <NA>         <NA>         NA 
      228 <NA>         <NA>         NA 
      229 <NA>         <NA>         NA 
      230 <NA>         <NA>         NA 
      231 <NA>         <NA>         NA 
      232 <NA>         <NA>         NA 
      233 <NA>         <NA>         NA 
      234 <NA>         <NA>         NA 
      235 <NA>         <NA>         NA 
      236 <NA>         <NA>         NA 
      237 <NA>         <NA>         NA 
      238 <NA>         <NA>         NA 
      239 <NA>         <NA>         NA 
      240 <NA>         <NA>         NA 
      241 <NA>         <NA>         NA 
      242 <NA>         <NA>         NA 
      243 <NA>         <NA>         NA 
      244 <NA>         <NA>         NA 
      245 <NA>         <NA>         NA 
      246 <NA>         <NA>         NA 
      247 <NA>         <NA>         NA 
      248 <NA>         <NA>         NA 
      249 <NA>         <NA>         NA 
      250 <NA>         <NA>         NA 
      251 <NA>         <NA>         NA 
      252 <NA>         <NA>         NA 
      253 <NA>         <NA>         NA 
      254 <NA>         <NA>         NA 
      255 <NA>         <NA>         NA 
      256 <NA>         <NA>         NA 
      257 <NA>         <NA>         NA 
      258 <NA>         <NA>         NA 
      259 <NA>         <NA>         NA 
      260 <NA>         <NA>         NA 
      261 <NA>         <NA>         NA 
      262 <NA>         <NA>         NA 
      263 <NA>         <NA>         NA 
      264 <NA>         <NA>         NA 
      265 <NA>         <NA>         NA 
      266 <NA>         <NA>         NA 
      267 <NA>         <NA>         NA 
      268 <NA>         <NA>         NA 
      269 <NA>         <NA>         NA 
      270 <NA>         <NA>         NA 
      271 <NA>         <NA>         NA 
      272 <NA>         <NA>         NA 
      273 <NA>         <NA>         NA 
      274 <NA>         <NA>         NA 
      275 <NA>         <NA>         NA 
      276 <NA>         <NA>         NA 
      277 <NA>         <NA>         NA 
      278 <NA>         <NA>         NA 
      279 <NA>         <NA>         NA 
      280 <NA>         <NA>         NA 
      281 <NA>         <NA>         NA 
      282 <NA>         <NA>         NA 
      283 <NA>         <NA>         NA 
      284 <NA>         <NA>         NA 
      285 <NA>         <NA>         NA 
      286 <NA>         <NA>         NA 
      287 <NA>         <NA>         NA 
      288 <NA>         <NA>         NA 
      289 <NA>         <NA>         NA 
      290 <NA>         <NA>         NA 
      291 <NA>         <NA>         NA 
      292 <NA>         <NA>         NA 
      293 <NA>         <NA>         NA 
      294 <NA>         <NA>         NA 
      295 <NA>         <NA>         NA 
      296 <NA>         <NA>         NA 
      297 <NA>         <NA>         NA 
      298 <NA>         <NA>         NA 
      299 <NA>         <NA>         NA 
      300 <NA>         <NA>         NA 
      301 <NA>         <NA>         NA 
      302 <NA>         <NA>         NA 
      303 <NA>         <NA>         NA 
      304 <NA>         <NA>         NA 
      305 <NA>         <NA>         NA 
      306 <NA>         <NA>         NA 
      307 <NA>         <NA>         NA 
      308 multinomial  data       -835.
      309 multinomial  data       -835.
      310 multinomial  data       -835.
      311 multinomial  data       -835.
      312 multinomial  data       -835.
      313 multinomial  data       -835.
      314 multinomial  data       -835.
      315 multinomial  data       -835.
      316 multinomial  data       -835.
      317 multinomial  data       -835.
      318 multinomial  data       -835.
      319 multinomial  data       -835.
      320 multinomial  data       -835.
      # i 13,927 more rows

