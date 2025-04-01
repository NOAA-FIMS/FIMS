# get_estimates() works with deterministic run

    Code
      print(dplyr::select(get_estimates(deterministic_results), -estimate,
      -uncertainty, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 6,439 x 15
      # Groups:   label [20]
          module_name module_id module_type label                        type  
          <chr>           <int> <chr>       <chr>                        <chr> 
        1 selectivity         1 Logistic    inflection_point             vector
        2 selectivity         1 Logistic    slope                        vector
        3 Fleet               1 fleet       log_q                        vector
        4 Fleet               1 fleet       log_Fmort                    vector
        5 Fleet               1 fleet       age_length_conversion_matrix vector
        6 Fleet               1 fleet       age_length_conversion_matrix vector
        7 Fleet               1 fleet       age_length_conversion_matrix vector
        8 Fleet               1 fleet       age_length_conversion_matrix vector
        9 Fleet               1 fleet       age_length_conversion_matrix vector
       10 Fleet               1 fleet       age_length_conversion_matrix vector
       11 Fleet               1 fleet       age_length_conversion_matrix vector
       12 Fleet               1 fleet       age_length_conversion_matrix vector
       13 Fleet               1 fleet       age_length_conversion_matrix vector
       14 Fleet               1 fleet       age_length_conversion_matrix vector
       15 Fleet               1 fleet       age_length_conversion_matrix vector
       16 Fleet               1 fleet       age_length_conversion_matrix vector
       17 Fleet               1 fleet       age_length_conversion_matrix vector
       18 Fleet               1 fleet       age_length_conversion_matrix vector
       19 Fleet               1 fleet       age_length_conversion_matrix vector
       20 Fleet               1 fleet       age_length_conversion_matrix vector
       21 Fleet               1 fleet       age_length_conversion_matrix vector
       22 Fleet               1 fleet       age_length_conversion_matrix vector
       23 Fleet               1 fleet       age_length_conversion_matrix vector
       24 Fleet               1 fleet       age_length_conversion_matrix vector
       25 Fleet               1 fleet       age_length_conversion_matrix vector
       26 Fleet               1 fleet       age_length_conversion_matrix vector
       27 Fleet               1 fleet       age_length_conversion_matrix vector
       28 Fleet               1 fleet       age_length_conversion_matrix vector
       29 Fleet               1 fleet       age_length_conversion_matrix vector
       30 Fleet               1 fleet       age_length_conversion_matrix vector
       31 Fleet               1 fleet       age_length_conversion_matrix vector
       32 Fleet               1 fleet       age_length_conversion_matrix vector
       33 Fleet               1 fleet       age_length_conversion_matrix vector
       34 Fleet               1 fleet       age_length_conversion_matrix vector
       35 Fleet               1 fleet       age_length_conversion_matrix vector
       36 Fleet               1 fleet       age_length_conversion_matrix vector
       37 Fleet               1 fleet       age_length_conversion_matrix vector
       38 Fleet               1 fleet       age_length_conversion_matrix vector
       39 Fleet               1 fleet       age_length_conversion_matrix vector
       40 Fleet               1 fleet       age_length_conversion_matrix vector
       41 Fleet               1 fleet       age_length_conversion_matrix vector
       42 Fleet               1 fleet       age_length_conversion_matrix vector
       43 Fleet               1 fleet       age_length_conversion_matrix vector
       44 Fleet               1 fleet       age_length_conversion_matrix vector
       45 Fleet               1 fleet       age_length_conversion_matrix vector
       46 Fleet               1 fleet       age_length_conversion_matrix vector
       47 Fleet               1 fleet       age_length_conversion_matrix vector
       48 Fleet               1 fleet       age_length_conversion_matrix vector
       49 Fleet               1 fleet       age_length_conversion_matrix vector
       50 Fleet               1 fleet       age_length_conversion_matrix vector
       51 Fleet               1 fleet       age_length_conversion_matrix vector
       52 Fleet               1 fleet       age_length_conversion_matrix vector
       53 Fleet               1 fleet       age_length_conversion_matrix vector
       54 Fleet               1 fleet       age_length_conversion_matrix vector
       55 Fleet               1 fleet       age_length_conversion_matrix vector
       56 Fleet               1 fleet       age_length_conversion_matrix vector
       57 Fleet               1 fleet       age_length_conversion_matrix vector
       58 Fleet               1 fleet       age_length_conversion_matrix vector
       59 Fleet               1 fleet       age_length_conversion_matrix vector
       60 Fleet               1 fleet       age_length_conversion_matrix vector
       61 Fleet               1 fleet       age_length_conversion_matrix vector
       62 Fleet               1 fleet       age_length_conversion_matrix vector
       63 Fleet               1 fleet       age_length_conversion_matrix vector
       64 Fleet               1 fleet       age_length_conversion_matrix vector
       65 Fleet               1 fleet       age_length_conversion_matrix vector
       66 Fleet               1 fleet       age_length_conversion_matrix vector
       67 Fleet               1 fleet       age_length_conversion_matrix vector
       68 Fleet               1 fleet       age_length_conversion_matrix vector
       69 Fleet               1 fleet       age_length_conversion_matrix vector
       70 Fleet               1 fleet       age_length_conversion_matrix vector
       71 Fleet               1 fleet       age_length_conversion_matrix vector
       72 Fleet               1 fleet       age_length_conversion_matrix vector
       73 Fleet               1 fleet       age_length_conversion_matrix vector
       74 Fleet               1 fleet       age_length_conversion_matrix vector
       75 Fleet               1 fleet       age_length_conversion_matrix vector
       76 Fleet               1 fleet       age_length_conversion_matrix vector
       77 Fleet               1 fleet       age_length_conversion_matrix vector
       78 Fleet               1 fleet       age_length_conversion_matrix vector
       79 Fleet               1 fleet       age_length_conversion_matrix vector
       80 Fleet               1 fleet       age_length_conversion_matrix vector
       81 Fleet               1 fleet       age_length_conversion_matrix vector
       82 Fleet               1 fleet       age_length_conversion_matrix vector
       83 Fleet               1 fleet       age_length_conversion_matrix vector
       84 Fleet               1 fleet       age_length_conversion_matrix vector
       85 Fleet               1 fleet       age_length_conversion_matrix vector
       86 Fleet               1 fleet       age_length_conversion_matrix vector
       87 Fleet               1 fleet       age_length_conversion_matrix vector
       88 Fleet               1 fleet       age_length_conversion_matrix vector
       89 Fleet               1 fleet       age_length_conversion_matrix vector
       90 Fleet               1 fleet       age_length_conversion_matrix vector
       91 Fleet               1 fleet       age_length_conversion_matrix vector
       92 Fleet               1 fleet       age_length_conversion_matrix vector
       93 Fleet               1 fleet       age_length_conversion_matrix vector
       94 Fleet               1 fleet       age_length_conversion_matrix vector
       95 Fleet               1 fleet       age_length_conversion_matrix vector
       96 Fleet               1 fleet       age_length_conversion_matrix vector
       97 Fleet               1 fleet       age_length_conversion_matrix vector
       98 Fleet               1 fleet       age_length_conversion_matrix vector
       99 Fleet               1 fleet       age_length_conversion_matrix vector
      100 Fleet               1 fleet       age_length_conversion_matrix vector
      101 Fleet               1 fleet       age_length_conversion_matrix vector
      102 Fleet               1 fleet       age_length_conversion_matrix vector
      103 Fleet               1 fleet       age_length_conversion_matrix vector
      104 Fleet               1 fleet       age_length_conversion_matrix vector
      105 Fleet               1 fleet       age_length_conversion_matrix vector
      106 Fleet               1 fleet       age_length_conversion_matrix vector
      107 Fleet               1 fleet       age_length_conversion_matrix vector
      108 Fleet               1 fleet       age_length_conversion_matrix vector
      109 Fleet               1 fleet       age_length_conversion_matrix vector
      110 Fleet               1 fleet       age_length_conversion_matrix vector
      111 Fleet               1 fleet       age_length_conversion_matrix vector
      112 Fleet               1 fleet       age_length_conversion_matrix vector
      113 Fleet               1 fleet       age_length_conversion_matrix vector
      114 Fleet               1 fleet       age_length_conversion_matrix vector
      115 Fleet               1 fleet       age_length_conversion_matrix vector
      116 Fleet               1 fleet       age_length_conversion_matrix vector
      117 Fleet               1 fleet       age_length_conversion_matrix vector
      118 Fleet               1 fleet       age_length_conversion_matrix vector
      119 Fleet               1 fleet       age_length_conversion_matrix vector
      120 Fleet               1 fleet       age_length_conversion_matrix vector
      121 Fleet               1 fleet       age_length_conversion_matrix vector
      122 Fleet               1 fleet       age_length_conversion_matrix vector
      123 Fleet               1 fleet       age_length_conversion_matrix vector
      124 Fleet               1 fleet       age_length_conversion_matrix vector
      125 Fleet               1 fleet       age_length_conversion_matrix vector
      126 Fleet               1 fleet       age_length_conversion_matrix vector
      127 Fleet               1 fleet       age_length_conversion_matrix vector
      128 Fleet               1 fleet       age_length_conversion_matrix vector
      129 Fleet               1 fleet       age_length_conversion_matrix vector
      130 Fleet               1 fleet       age_length_conversion_matrix vector
      131 Fleet               1 fleet       age_length_conversion_matrix vector
      132 Fleet               1 fleet       age_length_conversion_matrix vector
      133 Fleet               1 fleet       age_length_conversion_matrix vector
      134 Fleet               1 fleet       age_length_conversion_matrix vector
      135 Fleet               1 fleet       age_length_conversion_matrix vector
      136 Fleet               1 fleet       age_length_conversion_matrix vector
      137 Fleet               1 fleet       age_length_conversion_matrix vector
      138 Fleet               1 fleet       age_length_conversion_matrix vector
      139 Fleet               1 fleet       age_length_conversion_matrix vector
      140 Fleet               1 fleet       age_length_conversion_matrix vector
      141 Fleet               1 fleet       age_length_conversion_matrix vector
      142 Fleet               1 fleet       age_length_conversion_matrix vector
      143 Fleet               1 fleet       age_length_conversion_matrix vector
      144 Fleet               1 fleet       age_length_conversion_matrix vector
      145 Fleet               1 fleet       age_length_conversion_matrix vector
      146 Fleet               1 fleet       age_length_conversion_matrix vector
      147 Fleet               1 fleet       age_length_conversion_matrix vector
      148 Fleet               1 fleet       age_length_conversion_matrix vector
      149 Fleet               1 fleet       age_length_conversion_matrix vector
      150 Fleet               1 fleet       age_length_conversion_matrix vector
      151 Fleet               1 fleet       age_length_conversion_matrix vector
      152 Fleet               1 fleet       age_length_conversion_matrix vector
      153 Fleet               1 fleet       age_length_conversion_matrix vector
      154 Fleet               1 fleet       age_length_conversion_matrix vector
      155 Fleet               1 fleet       age_length_conversion_matrix vector
      156 Fleet               1 fleet       age_length_conversion_matrix vector
      157 Fleet               1 fleet       age_length_conversion_matrix vector
      158 Fleet               1 fleet       age_length_conversion_matrix vector
      159 Fleet               1 fleet       age_length_conversion_matrix vector
      160 Fleet               1 fleet       age_length_conversion_matrix vector
      161 Fleet               1 fleet       age_length_conversion_matrix vector
      162 Fleet               1 fleet       age_length_conversion_matrix vector
      163 Fleet               1 fleet       age_length_conversion_matrix vector
      164 Fleet               1 fleet       age_length_conversion_matrix vector
      165 Fleet               1 fleet       age_length_conversion_matrix vector
      166 Fleet               1 fleet       age_length_conversion_matrix vector
      167 Fleet               1 fleet       age_length_conversion_matrix vector
      168 Fleet               1 fleet       age_length_conversion_matrix vector
      169 Fleet               1 fleet       age_length_conversion_matrix vector
      170 Fleet               1 fleet       age_length_conversion_matrix vector
      171 Fleet               1 fleet       age_length_conversion_matrix vector
      172 Fleet               1 fleet       age_length_conversion_matrix vector
      173 Fleet               1 fleet       age_length_conversion_matrix vector
      174 Fleet               1 fleet       age_length_conversion_matrix vector
      175 Fleet               1 fleet       age_length_conversion_matrix vector
      176 Fleet               1 fleet       age_length_conversion_matrix vector
      177 Fleet               1 fleet       age_length_conversion_matrix vector
      178 Fleet               1 fleet       age_length_conversion_matrix vector
      179 Fleet               1 fleet       age_length_conversion_matrix vector
      180 Fleet               1 fleet       age_length_conversion_matrix vector
      181 Fleet               1 fleet       age_length_conversion_matrix vector
      182 Fleet               1 fleet       age_length_conversion_matrix vector
      183 Fleet               1 fleet       age_length_conversion_matrix vector
      184 Fleet               1 fleet       age_length_conversion_matrix vector
      185 Fleet               1 fleet       age_length_conversion_matrix vector
      186 Fleet               1 fleet       age_length_conversion_matrix vector
      187 Fleet               1 fleet       age_length_conversion_matrix vector
      188 Fleet               1 fleet       age_length_conversion_matrix vector
      189 Fleet               1 fleet       age_length_conversion_matrix vector
      190 Fleet               1 fleet       age_length_conversion_matrix vector
      191 Fleet               1 fleet       age_length_conversion_matrix vector
      192 Fleet               1 fleet       age_length_conversion_matrix vector
      193 Fleet               1 fleet       age_length_conversion_matrix vector
      194 Fleet               1 fleet       age_length_conversion_matrix vector
      195 Fleet               1 fleet       age_length_conversion_matrix vector
      196 Fleet               1 fleet       age_length_conversion_matrix vector
      197 Fleet               1 fleet       age_length_conversion_matrix vector
      198 Fleet               1 fleet       age_length_conversion_matrix vector
      199 Fleet               1 fleet       age_length_conversion_matrix vector
      200 Fleet               1 fleet       age_length_conversion_matrix vector
      201 Fleet               1 fleet       age_length_conversion_matrix vector
      202 Fleet               1 fleet       age_length_conversion_matrix vector
      203 Fleet               1 fleet       age_length_conversion_matrix vector
      204 Fleet               1 fleet       age_length_conversion_matrix vector
      205 Fleet               1 fleet       age_length_conversion_matrix vector
      206 Fleet               1 fleet       age_length_conversion_matrix vector
      207 Fleet               1 fleet       age_length_conversion_matrix vector
      208 Fleet               1 fleet       age_length_conversion_matrix vector
      209 Fleet               1 fleet       age_length_conversion_matrix vector
      210 Fleet               1 fleet       age_length_conversion_matrix vector
      211 Fleet               1 fleet       age_length_conversion_matrix vector
      212 Fleet               1 fleet       age_length_conversion_matrix vector
      213 Fleet               1 fleet       age_length_conversion_matrix vector
      214 Fleet               1 fleet       age_length_conversion_matrix vector
      215 Fleet               1 fleet       age_length_conversion_matrix vector
      216 Fleet               1 fleet       age_length_conversion_matrix vector
      217 Fleet               1 fleet       age_length_conversion_matrix vector
      218 Fleet               1 fleet       age_length_conversion_matrix vector
      219 Fleet               1 fleet       age_length_conversion_matrix vector
      220 Fleet               1 fleet       age_length_conversion_matrix vector
      221 Fleet               1 fleet       age_length_conversion_matrix vector
      222 Fleet               1 fleet       age_length_conversion_matrix vector
      223 Fleet               1 fleet       age_length_conversion_matrix vector
      224 Fleet               1 fleet       age_length_conversion_matrix vector
      225 Fleet               1 fleet       age_length_conversion_matrix vector
      226 Fleet               1 fleet       age_length_conversion_matrix vector
      227 Fleet               1 fleet       age_length_conversion_matrix vector
      228 Fleet               1 fleet       age_length_conversion_matrix vector
      229 Fleet               1 fleet       age_length_conversion_matrix vector
      230 Fleet               1 fleet       age_length_conversion_matrix vector
      231 Fleet               1 fleet       age_length_conversion_matrix vector
      232 Fleet               1 fleet       age_length_conversion_matrix vector
      233 Fleet               1 fleet       age_length_conversion_matrix vector
      234 Fleet               1 fleet       age_length_conversion_matrix vector
      235 Fleet               1 fleet       age_length_conversion_matrix vector
      236 Fleet               1 fleet       age_length_conversion_matrix vector
      237 Fleet               1 fleet       age_length_conversion_matrix vector
      238 Fleet               1 fleet       age_length_conversion_matrix vector
      239 Fleet               1 fleet       age_length_conversion_matrix vector
      240 Fleet               1 fleet       age_length_conversion_matrix vector
      241 Fleet               1 fleet       age_length_conversion_matrix vector
      242 Fleet               1 fleet       age_length_conversion_matrix vector
      243 Fleet               1 fleet       age_length_conversion_matrix vector
      244 Fleet               1 fleet       age_length_conversion_matrix vector
      245 Fleet               1 fleet       age_length_conversion_matrix vector
      246 Fleet               1 fleet       age_length_conversion_matrix vector
      247 Fleet               1 fleet       age_length_conversion_matrix vector
      248 Fleet               1 fleet       age_length_conversion_matrix vector
      249 Fleet               1 fleet       age_length_conversion_matrix vector
      250 Fleet               1 fleet       age_length_conversion_matrix vector
      251 Fleet               1 fleet       age_length_conversion_matrix vector
      252 Fleet               1 fleet       age_length_conversion_matrix vector
      253 Fleet               1 fleet       age_length_conversion_matrix vector
      254 Fleet               1 fleet       age_length_conversion_matrix vector
      255 Fleet               1 fleet       age_length_conversion_matrix vector
      256 Fleet               1 fleet       age_length_conversion_matrix vector
      257 Fleet               1 fleet       age_length_conversion_matrix vector
      258 Fleet               1 fleet       age_length_conversion_matrix vector
      259 Fleet               1 fleet       age_length_conversion_matrix vector
      260 Fleet               1 fleet       age_length_conversion_matrix vector
      261 Fleet               1 fleet       age_length_conversion_matrix vector
      262 Fleet               1 fleet       age_length_conversion_matrix vector
      263 Fleet               1 fleet       age_length_conversion_matrix vector
      264 Fleet               1 fleet       age_length_conversion_matrix vector
      265 Fleet               1 fleet       age_length_conversion_matrix vector
      266 Fleet               1 fleet       age_length_conversion_matrix vector
      267 Fleet               1 fleet       age_length_conversion_matrix vector
      268 Fleet               1 fleet       age_length_conversion_matrix vector
      269 Fleet               1 fleet       age_length_conversion_matrix vector
      270 Fleet               1 fleet       age_length_conversion_matrix vector
      271 Fleet               1 fleet       age_length_conversion_matrix vector
      272 Fleet               1 fleet       age_length_conversion_matrix vector
      273 Fleet               1 fleet       age_length_conversion_matrix vector
      274 Fleet               1 fleet       age_length_conversion_matrix vector
      275 Fleet               1 fleet       age_length_conversion_matrix vector
      276 Fleet               1 fleet       age_length_conversion_matrix vector
      277 Fleet               1 fleet       age_length_conversion_matrix vector
      278 Fleet               1 fleet       age_length_conversion_matrix vector
      279 Fleet               1 fleet       age_length_conversion_matrix vector
      280 Fleet               1 fleet       age_length_conversion_matrix vector
      281 Fleet               1 fleet       log_Fmort                    vector
      282 Fleet               1 fleet       log_Fmort                    vector
      283 Fleet               1 fleet       log_Fmort                    vector
      284 Fleet               1 fleet       log_Fmort                    vector
      285 Fleet               1 fleet       log_Fmort                    vector
      286 Fleet               1 fleet       log_Fmort                    vector
      287 Fleet               1 fleet       log_Fmort                    vector
      288 Fleet               1 fleet       log_Fmort                    vector
      289 Fleet               1 fleet       log_Fmort                    vector
      290 Fleet               1 fleet       log_Fmort                    vector
      291 Fleet               1 fleet       log_Fmort                    vector
      292 Fleet               1 fleet       log_Fmort                    vector
      293 Fleet               1 fleet       log_Fmort                    vector
      294 Fleet               1 fleet       log_Fmort                    vector
      295 Fleet               1 fleet       log_Fmort                    vector
      296 Fleet               1 fleet       log_Fmort                    vector
      297 Fleet               1 fleet       log_Fmort                    vector
      298 Fleet               1 fleet       log_Fmort                    vector
      299 Fleet               1 fleet       log_Fmort                    vector
      300 Fleet               1 fleet       log_Fmort                    vector
      301 Fleet               1 fleet       log_Fmort                    vector
      302 Fleet               1 fleet       log_Fmort                    vector
      303 Fleet               1 fleet       log_Fmort                    vector
      304 Fleet               1 fleet       log_Fmort                    vector
      305 Fleet               1 fleet       log_Fmort                    vector
      306 Fleet               1 fleet       log_Fmort                    vector
      307 Fleet               1 fleet       log_Fmort                    vector
      308 Fleet               1 fleet       log_Fmort                    vector
      309 Fleet               1 fleet       log_Fmort                    vector
      310 selectivity         2 Logistic    inflection_point             vector
      311 selectivity         2 Logistic    slope                        vector
      312 Fleet               2 fleet       log_q                        vector
      313 Fleet               2 fleet       log_Fmort                    vector
      314 Fleet               2 fleet       age_length_conversion_matrix vector
      315 Fleet               2 fleet       age_length_conversion_matrix vector
      316 Fleet               2 fleet       age_length_conversion_matrix vector
      317 Fleet               2 fleet       age_length_conversion_matrix vector
      318 Fleet               2 fleet       age_length_conversion_matrix vector
      319 Fleet               2 fleet       age_length_conversion_matrix vector
      320 Fleet               2 fleet       age_length_conversion_matrix vector
          type_id parameter_id fleet_name   age length  time   initial log_like
            <int>        <int> <chr>      <dbl>  <dbl> <int>     <dbl>    <dbl>
        1       1            1 <NA>          NA     NA    NA  2   e+ 0       NA
        2       2            2 <NA>          NA     NA    NA  1   e+ 0       NA
        3       3            3 <NA>          NA     NA    NA  0              NA
        4       4            4 <NA>          NA     NA    NA -4.66e+ 0       NA
        5       8            8 <NA>          NA     NA    NA  1.26e-16       NA
        6       8            9 <NA>          NA     NA    NA  8.39e-11       NA
        7       8           10 <NA>          NA     NA    NA  2.30e- 6       NA
        8       8           11 <NA>          NA     NA    NA  2.74e- 3       NA
        9       8           12 <NA>          NA     NA    NA  1.63e- 1       NA
       10       8           13 <NA>          NA     NA    NA  6.32e- 1       NA
       11       8           14 <NA>          NA     NA    NA  1.98e- 1       NA
       12       8           15 <NA>          NA     NA    NA  4.13e- 3       NA
       13       8           16 <NA>          NA     NA    NA  4.36e- 6       NA
       14       8           17 <NA>          NA     NA    NA  2.02e-10       NA
       15       8           18 <NA>          NA     NA    NA  3.33e-16       NA
       16       8           19 <NA>          NA     NA    NA  0              NA
       17       8           20 <NA>          NA     NA    NA  0              NA
       18       8           21 <NA>          NA     NA    NA  0              NA
       19       8           22 <NA>          NA     NA    NA  0              NA
       20       8           23 <NA>          NA     NA    NA  0              NA
       21       8           24 <NA>          NA     NA    NA  0              NA
       22       8           25 <NA>          NA     NA    NA  0              NA
       23       8           26 <NA>          NA     NA    NA  0              NA
       24       8           27 <NA>          NA     NA    NA  0              NA
       25       8           28 <NA>          NA     NA    NA  0              NA
       26       8           29 <NA>          NA     NA    NA  0              NA
       27       8           30 <NA>          NA     NA    NA  0              NA
       28       8           31 <NA>          NA     NA    NA  3.27e-18       NA
       29       8           32 <NA>          NA     NA    NA  2.15e-13       NA
       30       8           33 <NA>          NA     NA    NA  2.20e- 9       NA
       31       8           34 <NA>          NA     NA    NA  3.54e- 6       NA
       32       8           35 <NA>          NA     NA    NA  9.19e- 4       NA
       33       8           36 <NA>          NA     NA    NA  4.03e- 2       NA
       34       8           37 <NA>          NA     NA    NA  3.18e- 1       NA
       35       8           38 <NA>          NA     NA    NA  4.86e- 1       NA
       36       8           39 <NA>          NA     NA    NA  1.46e- 1       NA
       37       8           40 <NA>          NA     NA    NA  8.23e- 3       NA
       38       8           41 <NA>          NA     NA    NA  8.08e- 5       NA
       39       8           42 <NA>          NA     NA    NA  1.31e- 7       NA
       40       8           43 <NA>          NA     NA    NA  3.37e-11       NA
       41       8           44 <NA>          NA     NA    NA  1.33e-15       NA
       42       8           45 <NA>          NA     NA    NA  0              NA
       43       8           46 <NA>          NA     NA    NA  0              NA
       44       8           47 <NA>          NA     NA    NA  0              NA
       45       8           48 <NA>          NA     NA    NA  0              NA
       46       8           49 <NA>          NA     NA    NA  0              NA
       47       8           50 <NA>          NA     NA    NA  0              NA
       48       8           51 <NA>          NA     NA    NA  0              NA
       49       8           52 <NA>          NA     NA    NA  0              NA
       50       8           53 <NA>          NA     NA    NA  0              NA
       51       8           54 <NA>          NA     NA    NA  4.35e-19       NA
       52       8           55 <NA>          NA     NA    NA  6.73e-15       NA
       53       8           56 <NA>          NA     NA    NA  2.84e-11       NA
       54       8           57 <NA>          NA     NA    NA  3.28e- 8       NA
       55       8           58 <NA>          NA     NA    NA  1.05e- 5       NA
       56       8           59 <NA>          NA     NA    NA  9.44e- 4       NA
       57       8           60 <NA>          NA     NA    NA  2.44e- 2       NA
       58       8           61 <NA>          NA     NA    NA  1.85e- 1       NA
       59       8           62 <NA>          NA     NA    NA  4.24e- 1       NA
       60       8           63 <NA>          NA     NA    NA  2.98e- 1       NA
       61       8           64 <NA>          NA     NA    NA  6.36e- 2       NA
       62       8           65 <NA>          NA     NA    NA  4.04e- 3       NA
       63       8           66 <NA>          NA     NA    NA  7.43e- 5       NA
       64       8           67 <NA>          NA     NA    NA  3.87e- 7       NA
       65       8           68 <NA>          NA     NA    NA  5.62e-10       NA
       66       8           69 <NA>          NA     NA    NA  2.24e-13       NA
       67       8           70 <NA>          NA     NA    NA  0              NA
       68       8           71 <NA>          NA     NA    NA  0              NA
       69       8           72 <NA>          NA     NA    NA  0              NA
       70       8           73 <NA>          NA     NA    NA  0              NA
       71       8           74 <NA>          NA     NA    NA  0              NA
       72       8           75 <NA>          NA     NA    NA  0              NA
       73       8           76 <NA>          NA     NA    NA  0              NA
       74       8           77 <NA>          NA     NA    NA  1.23e-19       NA
       75       8           78 <NA>          NA     NA    NA  7.29e-16       NA
       76       8           79 <NA>          NA     NA    NA  1.58e-12       NA
       77       8           80 <NA>          NA     NA    NA  1.25e- 9       NA
       78       8           81 <NA>          NA     NA    NA  3.68e- 7       NA
       79       8           82 <NA>          NA     NA    NA  4.02e- 5       NA
       80       8           83 <NA>          NA     NA    NA  1.65e- 3       NA
       81       8           84 <NA>          NA     NA    NA  2.56e- 2       NA
       82       8           85 <NA>          NA     NA    NA  1.54e- 1       NA
       83       8           86 <NA>          NA     NA    NA  3.58e- 1       NA
       84       8           87 <NA>          NA     NA    NA  3.27e- 1       NA
       85       8           88 <NA>          NA     NA    NA  1.17e- 1       NA
       86       8           89 <NA>          NA     NA    NA  1.62e- 2       NA
       87       8           90 <NA>          NA     NA    NA  8.65e- 4       NA
       88       8           91 <NA>          NA     NA    NA  1.75e- 5       NA
       89       8           92 <NA>          NA     NA    NA  1.32e- 7       NA
       90       8           93 <NA>          NA     NA    NA  3.71e-10       NA
       91       8           94 <NA>          NA     NA    NA  3.85e-13       NA
       92       8           95 <NA>          NA     NA    NA  1.11e-16       NA
       93       8           96 <NA>          NA     NA    NA  0              NA
       94       8           97 <NA>          NA     NA    NA  0              NA
       95       8           98 <NA>          NA     NA    NA  0              NA
       96       8           99 <NA>          NA     NA    NA  0              NA
       97       8          100 <NA>          NA     NA    NA  5.27e-20       NA
       98       8          101 <NA>          NA     NA    NA  1.59e-16       NA
       99       8          102 <NA>          NA     NA    NA  2.09e-13       NA
      100       8          103 <NA>          NA     NA    NA  1.20e-10       NA
      101       8          104 <NA>          NA     NA    NA  3.04e- 8       NA
      102       8          105 <NA>          NA     NA    NA  3.38e- 6       NA
      103       8          106 <NA>          NA     NA    NA  1.67e- 4       NA
      104       8          107 <NA>          NA     NA    NA  3.67e- 3       NA
      105       8          108 <NA>          NA     NA    NA  3.63e- 2       NA
      106       8          109 <NA>          NA     NA    NA  1.63e- 1       NA
      107       8          110 <NA>          NA     NA    NA  3.31e- 1       NA
      108       8          111 <NA>          NA     NA    NA  3.08e- 1       NA
      109       8          112 <NA>          NA     NA    NA  1.31e- 1       NA
      110       8          113 <NA>          NA     NA    NA  2.52e- 2       NA
      111       8          114 <NA>          NA     NA    NA  2.20e- 3       NA
      112       8          115 <NA>          NA     NA    NA  8.62e- 5       NA
      113       8          116 <NA>          NA     NA    NA  1.50e- 6       NA
      114       8          117 <NA>          NA     NA    NA  1.16e- 8       NA
      115       8          118 <NA>          NA     NA    NA  3.94e-11       NA
      116       8          119 <NA>          NA     NA    NA  5.88e-14       NA
      117       8          120 <NA>          NA     NA    NA  0              NA
      118       8          121 <NA>          NA     NA    NA  0              NA
      119       8          122 <NA>          NA     NA    NA  0              NA
      120       8          123 <NA>          NA     NA    NA  2.88e-20       NA
      121       8          124 <NA>          NA     NA    NA  5.32e-17       NA
      122       8          125 <NA>          NA     NA    NA  4.81e-14       NA
      123       8          126 <NA>          NA     NA    NA  2.13e-11       NA
      124       8          127 <NA>          NA     NA    NA  4.62e- 9       NA
      125       8          128 <NA>          NA     NA    NA  4.94e- 7       NA
      126       8          129 <NA>          NA     NA    NA  2.61e- 5       NA
      127       8          130 <NA>          NA     NA    NA  6.86e- 4       NA
      128       8          131 <NA>          NA     NA    NA  8.98e- 3       NA
      129       8          132 <NA>          NA     NA    NA  5.89e- 2       NA
      130       8          133 <NA>          NA     NA    NA  1.94e- 1       NA
      131       8          134 <NA>          NA     NA    NA  3.23e- 1       NA
      132       8          135 <NA>          NA     NA    NA  2.72e- 1       NA
      133       8          136 <NA>          NA     NA    NA  1.15e- 1       NA
      134       8          137 <NA>          NA     NA    NA  2.47e- 2       NA
      135       8          138 <NA>          NA     NA    NA  2.66e- 3       NA
      136       8          139 <NA>          NA     NA    NA  1.43e- 4       NA
      137       8          140 <NA>          NA     NA    NA  3.81e- 6       NA
      138       8          141 <NA>          NA     NA    NA  5.04e- 8       NA
      139       8          142 <NA>          NA     NA    NA  3.29e-10       NA
      140       8          143 <NA>          NA     NA    NA  1.06e-12       NA
      141       8          144 <NA>          NA     NA    NA  1.67e-15       NA
      142       8          145 <NA>          NA     NA    NA  0              NA
      143       8          146 <NA>          NA     NA    NA  1.85e-20       NA
      144       8          147 <NA>          NA     NA    NA  2.36e-17       NA
      145       8          148 <NA>          NA     NA    NA  1.60e-14       NA
      146       8          149 <NA>          NA     NA    NA  5.73e-12       NA
      147       8          150 <NA>          NA     NA    NA  1.09e- 9       NA
      148       8          151 <NA>          NA     NA    NA  1.10e- 7       NA
      149       8          152 <NA>          NA     NA    NA  5.93e- 6       NA
      150       8          153 <NA>          NA     NA    NA  1.71e- 4       NA
      151       8          154 <NA>          NA     NA    NA  2.63e- 3       NA
      152       8          155 <NA>          NA     NA    NA  2.18e- 2       NA
      153       8          156 <NA>          NA     NA    NA  9.79e- 2       NA
      154       8          157 <NA>          NA     NA    NA  2.37e- 1       NA
      155       8          158 <NA>          NA     NA    NA  3.12e- 1       NA
      156       8          159 <NA>          NA     NA    NA  2.22e- 1       NA
      157       8          160 <NA>          NA     NA    NA  8.59e- 2       NA
      158       8          161 <NA>          NA     NA    NA  1.80e- 2       NA
      159       8          162 <NA>          NA     NA    NA  2.03e- 3       NA
      160       8          163 <NA>          NA     NA    NA  1.23e- 4       NA
      161       8          164 <NA>          NA     NA    NA  4.00e- 6       NA
      162       8          165 <NA>          NA     NA    NA  6.96e- 8       NA
      163       8          166 <NA>          NA     NA    NA  6.44e-10       NA
      164       8          167 <NA>          NA     NA    NA  3.17e-12       NA
      165       8          168 <NA>          NA     NA    NA  8.33e-15       NA
      166       8          169 <NA>          NA     NA    NA  1.32e-20       NA
      167       8          170 <NA>          NA     NA    NA  1.28e-17       NA
      168       8          171 <NA>          NA     NA    NA  6.90e-15       NA
      169       8          172 <NA>          NA     NA    NA  2.09e-12       NA
      170       8          173 <NA>          NA     NA    NA  3.54e-10       NA
      171       8          174 <NA>          NA     NA    NA  3.37e- 8       NA
      172       8          175 <NA>          NA     NA    NA  1.81e- 6       NA
      173       8          176 <NA>          NA     NA    NA  5.45e- 5       NA
      174       8          177 <NA>          NA     NA    NA  9.31e- 4       NA
      175       8          178 <NA>          NA     NA    NA  9.00e- 3       NA
      176       8          179 <NA>          NA     NA    NA  4.95e- 2       NA
      177       8          180 <NA>          NA     NA    NA  1.55e- 1       NA
      178       8          181 <NA>          NA     NA    NA  2.76e- 1       NA
      179       8          182 <NA>          NA     NA    NA  2.81e- 1       NA
      180       8          183 <NA>          NA     NA    NA  1.63e- 1       NA
      181       8          184 <NA>          NA     NA    NA  5.42e- 2       NA
      182       8          185 <NA>          NA     NA    NA  1.02e- 2       NA
      183       8          186 <NA>          NA     NA    NA  1.10e- 3       NA
      184       8          187 <NA>          NA     NA    NA  6.66e- 5       NA
      185       8          188 <NA>          NA     NA    NA  2.29e- 6       NA
      186       8          189 <NA>          NA     NA    NA  4.44e- 8       NA
      187       8          190 <NA>          NA     NA    NA  4.83e-10       NA
      188       8          191 <NA>          NA     NA    NA  2.97e-12       NA
      189       8          192 <NA>          NA     NA    NA  1.02e-20       NA
      190       8          193 <NA>          NA     NA    NA  7.91e-18       NA
      191       8          194 <NA>          NA     NA    NA  3.58e-15       NA
      192       8          195 <NA>          NA     NA    NA  9.46e-13       NA
      193       8          196 <NA>          NA     NA    NA  1.46e-10       NA
      194       8          197 <NA>          NA     NA    NA  1.31e- 8       NA
      195       8          198 <NA>          NA     NA    NA  6.94e- 7       NA
      196       8          199 <NA>          NA     NA    NA  2.15e- 5       NA
      197       8          200 <NA>          NA     NA    NA  3.91e- 4       NA
      198       8          201 <NA>          NA     NA    NA  4.19e- 3       NA
      199       8          202 <NA>          NA     NA    NA  2.65e- 2       NA
      200       8          203 <NA>          NA     NA    NA  9.92e- 2       NA
      201       8          204 <NA>          NA     NA    NA  2.20e- 1       NA
      202       8          205 <NA>          NA     NA    NA  2.88e- 1       NA
      203       8          206 <NA>          NA     NA    NA  2.25e- 1       NA
      204       8          207 <NA>          NA     NA    NA  1.04e- 1       NA
      205       8          208 <NA>          NA     NA    NA  2.83e- 2       NA
      206       8          209 <NA>          NA     NA    NA  4.58e- 3       NA
      207       8          210 <NA>          NA     NA    NA  4.37e- 4       NA
      208       8          211 <NA>          NA     NA    NA  2.46e- 5       NA
      209       8          212 <NA>          NA     NA    NA  8.11e- 7       NA
      210       8          213 <NA>          NA     NA    NA  1.57e- 8       NA
      211       8          214 <NA>          NA     NA    NA  1.80e-10       NA
      212       8          215 <NA>          NA     NA    NA  8.31e-21       NA
      213       8          216 <NA>          NA     NA    NA  5.43e-18       NA
      214       8          217 <NA>          NA     NA    NA  2.14e-15       NA
      215       8          218 <NA>          NA     NA    NA  5.06e-13       NA
      216       8          219 <NA>          NA     NA    NA  7.20e-11       NA
      217       8          220 <NA>          NA     NA    NA  6.18e- 9       NA
      218       8          221 <NA>          NA     NA    NA  3.20e- 7       NA
      219       8          222 <NA>          NA     NA    NA  1.00e- 5       NA
      220       8          223 <NA>          NA     NA    NA  1.91e- 4       NA
      221       8          224 <NA>          NA     NA    NA  2.19e- 3       NA
      222       8          225 <NA>          NA     NA    NA  1.53e- 2       NA
      223       8          226 <NA>          NA     NA    NA  6.53e- 2       NA
      224       8          227 <NA>          NA     NA    NA  1.69e- 1       NA
      225       8          228 <NA>          NA     NA    NA  2.68e- 1       NA
      226       8          229 <NA>          NA     NA    NA  2.59e- 1       NA
      227       8          230 <NA>          NA     NA    NA  1.53e- 1       NA
      228       8          231 <NA>          NA     NA    NA  5.48e- 2       NA
      229       8          232 <NA>          NA     NA    NA  1.20e- 2       NA
      230       8          233 <NA>          NA     NA    NA  1.60e- 3       NA
      231       8          234 <NA>          NA     NA    NA  1.30e- 4       NA
      232       8          235 <NA>          NA     NA    NA  6.37e- 6       NA
      233       8          236 <NA>          NA     NA    NA  1.89e- 7       NA
      234       8          237 <NA>          NA     NA    NA  3.44e- 9       NA
      235       8          238 <NA>          NA     NA    NA  7.07e-21       NA
      236       8          239 <NA>          NA     NA    NA  4.03e-18       NA
      237       8          240 <NA>          NA     NA    NA  1.42e-15       NA
      238       8          241 <NA>          NA     NA    NA  3.06e-13       NA
      239       8          242 <NA>          NA     NA    NA  4.09e-11       NA
      240       8          243 <NA>          NA     NA    NA  3.36e- 9       NA
      241       8          244 <NA>          NA     NA    NA  1.71e- 7       NA
      242       8          245 <NA>          NA     NA    NA  5.39e- 6       NA
      243       8          246 <NA>          NA     NA    NA  1.05e- 4       NA
      244       8          247 <NA>          NA     NA    NA  1.27e- 3       NA
      245       8          248 <NA>          NA     NA    NA  9.57e- 3       NA
      246       8          249 <NA>          NA     NA    NA  4.48e- 2       NA
      247       8          250 <NA>          NA     NA    NA  1.31e- 1       NA
      248       8          251 <NA>          NA     NA    NA  2.38e- 1       NA
      249       8          252 <NA>          NA     NA    NA  2.71e- 1       NA
      250       8          253 <NA>          NA     NA    NA  1.92e- 1       NA
      251       8          254 <NA>          NA     NA    NA  8.49e- 2       NA
      252       8          255 <NA>          NA     NA    NA  2.34e- 2       NA
      253       8          256 <NA>          NA     NA    NA  4.02e- 3       NA
      254       8          257 <NA>          NA     NA    NA  4.30e- 4       NA
      255       8          258 <NA>          NA     NA    NA  2.85e- 5       NA
      256       8          259 <NA>          NA     NA    NA  1.17e- 6       NA
      257       8          260 <NA>          NA     NA    NA  3.04e- 8       NA
      258       8          261 <NA>          NA     NA    NA  6.22e-21       NA
      259       8          262 <NA>          NA     NA    NA  3.17e-18       NA
      260       8          263 <NA>          NA     NA    NA  1.02e-15       NA
      261       8          264 <NA>          NA     NA    NA  2.04e-13       NA
      262       8          265 <NA>          NA     NA    NA  2.58e-11       NA
      263       8          266 <NA>          NA     NA    NA  2.05e- 9       NA
      264       8          267 <NA>          NA     NA    NA  1.03e- 7       NA
      265       8          268 <NA>          NA     NA    NA  3.23e- 6       NA
      266       8          269 <NA>          NA     NA    NA  6.43e- 5       NA
      267       8          270 <NA>          NA     NA    NA  8.07e- 4       NA
      268       8          271 <NA>          NA     NA    NA  6.41e- 3       NA
      269       8          272 <NA>          NA     NA    NA  3.23e- 2       NA
      270       8          273 <NA>          NA     NA    NA  1.03e- 1       NA
      271       8          274 <NA>          NA     NA    NA  2.09e- 1       NA
      272       8          275 <NA>          NA     NA    NA  2.69e- 1       NA
      273       8          276 <NA>          NA     NA    NA  2.20e- 1       NA
      274       8          277 <NA>          NA     NA    NA  1.14e- 1       NA
      275       8          278 <NA>          NA     NA    NA  3.76e- 2       NA
      276       8          279 <NA>          NA     NA    NA  7.85e- 3       NA
      277       8          280 <NA>          NA     NA    NA  1.04e- 3       NA
      278       8          281 <NA>          NA     NA    NA  8.70e- 5       NA
      279       8          282 <NA>          NA     NA    NA  4.61e- 6       NA
      280       8          283 <NA>          NA     NA    NA  1.57e- 7       NA
      281       4          284 <NA>          NA     NA    NA -3.60e+ 0       NA
      282       4          285 <NA>          NA     NA    NA -3.10e+ 0       NA
      283       4          286 <NA>          NA     NA    NA -2.80e+ 0       NA
      284       4          287 <NA>          NA     NA    NA -3.02e+ 0       NA
      285       4          288 <NA>          NA     NA    NA -2.44e+ 0       NA
      286       4          289 <NA>          NA     NA    NA -2.43e+ 0       NA
      287       4          290 <NA>          NA     NA    NA -1.68e+ 0       NA
      288       4          291 <NA>          NA     NA    NA -2.22e+ 0       NA
      289       4          292 <NA>          NA     NA    NA -2.02e+ 0       NA
      290       4          293 <NA>          NA     NA    NA -1.89e+ 0       NA
      291       4          294 <NA>          NA     NA    NA -1.82e+ 0       NA
      292       4          295 <NA>          NA     NA    NA -2.15e+ 0       NA
      293       4          296 <NA>          NA     NA    NA -1.78e+ 0       NA
      294       4          297 <NA>          NA     NA    NA -1.71e+ 0       NA
      295       4          298 <NA>          NA     NA    NA -1.82e+ 0       NA
      296       4          299 <NA>          NA     NA    NA -1.16e+ 0       NA
      297       4          300 <NA>          NA     NA    NA -1.36e+ 0       NA
      298       4          301 <NA>          NA     NA    NA -1.37e+ 0       NA
      299       4          302 <NA>          NA     NA    NA -1.38e+ 0       NA
      300       4          303 <NA>          NA     NA    NA -1.05e+ 0       NA
      301       4          304 <NA>          NA     NA    NA -1.37e+ 0       NA
      302       4          305 <NA>          NA     NA    NA -8.71e- 1       NA
      303       4          306 <NA>          NA     NA    NA -1.06e+ 0       NA
      304       4          307 <NA>          NA     NA    NA -1.07e+ 0       NA
      305       4          308 <NA>          NA     NA    NA -1.16e+ 0       NA
      306       4          309 <NA>          NA     NA    NA -1.18e+ 0       NA
      307       4          310 <NA>          NA     NA    NA -8.40e- 1       NA
      308       4          311 <NA>          NA     NA    NA -1.11e+ 0       NA
      309       4          312 <NA>          NA     NA    NA -6.94e- 1       NA
      310      16          349 <NA>          NA     NA    NA  1.5 e+ 0       NA
      311      17          350 <NA>          NA     NA    NA  2   e+ 0       NA
      312      18          351 <NA>          NA     NA    NA -1.49e+ 1       NA
      313      19          352 <NA>          NA     NA    NA  0              NA
      314      23          356 <NA>          NA     NA    NA  1.26e-16       NA
      315      23          357 <NA>          NA     NA    NA  8.39e-11       NA
      316      23          358 <NA>          NA     NA    NA  2.30e- 6       NA
      317      23          359 <NA>          NA     NA    NA  2.74e- 3       NA
      318      23          360 <NA>          NA     NA    NA  1.63e- 1       NA
      319      23          361 <NA>          NA     NA    NA  6.32e- 1       NA
      320      23          362 <NA>          NA     NA    NA  1.98e- 1       NA
          log_like_cv estimated
                <dbl> <lgl>    
        1          NA FALSE    
        2          NA FALSE    
        3          NA FALSE    
        4          NA FALSE    
        5          NA FALSE    
        6          NA FALSE    
        7          NA FALSE    
        8          NA FALSE    
        9          NA FALSE    
       10          NA FALSE    
       11          NA FALSE    
       12          NA FALSE    
       13          NA FALSE    
       14          NA FALSE    
       15          NA FALSE    
       16          NA FALSE    
       17          NA FALSE    
       18          NA FALSE    
       19          NA FALSE    
       20          NA FALSE    
       21          NA FALSE    
       22          NA FALSE    
       23          NA FALSE    
       24          NA FALSE    
       25          NA FALSE    
       26          NA FALSE    
       27          NA FALSE    
       28          NA FALSE    
       29          NA FALSE    
       30          NA FALSE    
       31          NA FALSE    
       32          NA FALSE    
       33          NA FALSE    
       34          NA FALSE    
       35          NA FALSE    
       36          NA FALSE    
       37          NA FALSE    
       38          NA FALSE    
       39          NA FALSE    
       40          NA FALSE    
       41          NA FALSE    
       42          NA FALSE    
       43          NA FALSE    
       44          NA FALSE    
       45          NA FALSE    
       46          NA FALSE    
       47          NA FALSE    
       48          NA FALSE    
       49          NA FALSE    
       50          NA FALSE    
       51          NA FALSE    
       52          NA FALSE    
       53          NA FALSE    
       54          NA FALSE    
       55          NA FALSE    
       56          NA FALSE    
       57          NA FALSE    
       58          NA FALSE    
       59          NA FALSE    
       60          NA FALSE    
       61          NA FALSE    
       62          NA FALSE    
       63          NA FALSE    
       64          NA FALSE    
       65          NA FALSE    
       66          NA FALSE    
       67          NA FALSE    
       68          NA FALSE    
       69          NA FALSE    
       70          NA FALSE    
       71          NA FALSE    
       72          NA FALSE    
       73          NA FALSE    
       74          NA FALSE    
       75          NA FALSE    
       76          NA FALSE    
       77          NA FALSE    
       78          NA FALSE    
       79          NA FALSE    
       80          NA FALSE    
       81          NA FALSE    
       82          NA FALSE    
       83          NA FALSE    
       84          NA FALSE    
       85          NA FALSE    
       86          NA FALSE    
       87          NA FALSE    
       88          NA FALSE    
       89          NA FALSE    
       90          NA FALSE    
       91          NA FALSE    
       92          NA FALSE    
       93          NA FALSE    
       94          NA FALSE    
       95          NA FALSE    
       96          NA FALSE    
       97          NA FALSE    
       98          NA FALSE    
       99          NA FALSE    
      100          NA FALSE    
      101          NA FALSE    
      102          NA FALSE    
      103          NA FALSE    
      104          NA FALSE    
      105          NA FALSE    
      106          NA FALSE    
      107          NA FALSE    
      108          NA FALSE    
      109          NA FALSE    
      110          NA FALSE    
      111          NA FALSE    
      112          NA FALSE    
      113          NA FALSE    
      114          NA FALSE    
      115          NA FALSE    
      116          NA FALSE    
      117          NA FALSE    
      118          NA FALSE    
      119          NA FALSE    
      120          NA FALSE    
      121          NA FALSE    
      122          NA FALSE    
      123          NA FALSE    
      124          NA FALSE    
      125          NA FALSE    
      126          NA FALSE    
      127          NA FALSE    
      128          NA FALSE    
      129          NA FALSE    
      130          NA FALSE    
      131          NA FALSE    
      132          NA FALSE    
      133          NA FALSE    
      134          NA FALSE    
      135          NA FALSE    
      136          NA FALSE    
      137          NA FALSE    
      138          NA FALSE    
      139          NA FALSE    
      140          NA FALSE    
      141          NA FALSE    
      142          NA FALSE    
      143          NA FALSE    
      144          NA FALSE    
      145          NA FALSE    
      146          NA FALSE    
      147          NA FALSE    
      148          NA FALSE    
      149          NA FALSE    
      150          NA FALSE    
      151          NA FALSE    
      152          NA FALSE    
      153          NA FALSE    
      154          NA FALSE    
      155          NA FALSE    
      156          NA FALSE    
      157          NA FALSE    
      158          NA FALSE    
      159          NA FALSE    
      160          NA FALSE    
      161          NA FALSE    
      162          NA FALSE    
      163          NA FALSE    
      164          NA FALSE    
      165          NA FALSE    
      166          NA FALSE    
      167          NA FALSE    
      168          NA FALSE    
      169          NA FALSE    
      170          NA FALSE    
      171          NA FALSE    
      172          NA FALSE    
      173          NA FALSE    
      174          NA FALSE    
      175          NA FALSE    
      176          NA FALSE    
      177          NA FALSE    
      178          NA FALSE    
      179          NA FALSE    
      180          NA FALSE    
      181          NA FALSE    
      182          NA FALSE    
      183          NA FALSE    
      184          NA FALSE    
      185          NA FALSE    
      186          NA FALSE    
      187          NA FALSE    
      188          NA FALSE    
      189          NA FALSE    
      190          NA FALSE    
      191          NA FALSE    
      192          NA FALSE    
      193          NA FALSE    
      194          NA FALSE    
      195          NA FALSE    
      196          NA FALSE    
      197          NA FALSE    
      198          NA FALSE    
      199          NA FALSE    
      200          NA FALSE    
      201          NA FALSE    
      202          NA FALSE    
      203          NA FALSE    
      204          NA FALSE    
      205          NA FALSE    
      206          NA FALSE    
      207          NA FALSE    
      208          NA FALSE    
      209          NA FALSE    
      210          NA FALSE    
      211          NA FALSE    
      212          NA FALSE    
      213          NA FALSE    
      214          NA FALSE    
      215          NA FALSE    
      216          NA FALSE    
      217          NA FALSE    
      218          NA FALSE    
      219          NA FALSE    
      220          NA FALSE    
      221          NA FALSE    
      222          NA FALSE    
      223          NA FALSE    
      224          NA FALSE    
      225          NA FALSE    
      226          NA FALSE    
      227          NA FALSE    
      228          NA FALSE    
      229          NA FALSE    
      230          NA FALSE    
      231          NA FALSE    
      232          NA FALSE    
      233          NA FALSE    
      234          NA FALSE    
      235          NA FALSE    
      236          NA FALSE    
      237          NA FALSE    
      238          NA FALSE    
      239          NA FALSE    
      240          NA FALSE    
      241          NA FALSE    
      242          NA FALSE    
      243          NA FALSE    
      244          NA FALSE    
      245          NA FALSE    
      246          NA FALSE    
      247          NA FALSE    
      248          NA FALSE    
      249          NA FALSE    
      250          NA FALSE    
      251          NA FALSE    
      252          NA FALSE    
      253          NA FALSE    
      254          NA FALSE    
      255          NA FALSE    
      256          NA FALSE    
      257          NA FALSE    
      258          NA FALSE    
      259          NA FALSE    
      260          NA FALSE    
      261          NA FALSE    
      262          NA FALSE    
      263          NA FALSE    
      264          NA FALSE    
      265          NA FALSE    
      266          NA FALSE    
      267          NA FALSE    
      268          NA FALSE    
      269          NA FALSE    
      270          NA FALSE    
      271          NA FALSE    
      272          NA FALSE    
      273          NA FALSE    
      274          NA FALSE    
      275          NA FALSE    
      276          NA FALSE    
      277          NA FALSE    
      278          NA FALSE    
      279          NA FALSE    
      280          NA FALSE    
      281          NA FALSE    
      282          NA FALSE    
      283          NA FALSE    
      284          NA FALSE    
      285          NA FALSE    
      286          NA FALSE    
      287          NA FALSE    
      288          NA FALSE    
      289          NA FALSE    
      290          NA FALSE    
      291          NA FALSE    
      292          NA FALSE    
      293          NA FALSE    
      294          NA FALSE    
      295          NA FALSE    
      296          NA FALSE    
      297          NA FALSE    
      298          NA FALSE    
      299          NA FALSE    
      300          NA FALSE    
      301          NA FALSE    
      302          NA FALSE    
      303          NA FALSE    
      304          NA FALSE    
      305          NA FALSE    
      306          NA FALSE    
      307          NA FALSE    
      308          NA FALSE    
      309          NA FALSE    
      310          NA FALSE    
      311          NA FALSE    
      312          NA FALSE    
      313          NA FALSE    
      314          NA FALSE    
      315          NA FALSE    
      316          NA FALSE    
      317          NA FALSE    
      318          NA FALSE    
      319          NA FALSE    
      320          NA FALSE    
      # i 6,119 more rows

# get_estimates() works with estimation run

    Code
      print(dplyr::select(get_estimates(readRDS(fit_files[[1]])), -estimate,
      -uncertainty, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 6,530 x 15
      # Groups:   label [23]
          module_name module_id module_type label                        type  
          <chr>           <int> <chr>       <chr>                        <chr> 
        1 selectivity         1 Logistic    inflection_point             vector
        2 selectivity         1 Logistic    slope                        vector
        3 Fleet               1 fleet       log_q                        vector
        4 Fleet               1 fleet       log_Fmort                    vector
        5 Fleet               1 fleet       age_length_conversion_matrix vector
        6 Fleet               1 fleet       age_length_conversion_matrix vector
        7 Fleet               1 fleet       age_length_conversion_matrix vector
        8 Fleet               1 fleet       age_length_conversion_matrix vector
        9 Fleet               1 fleet       age_length_conversion_matrix vector
       10 Fleet               1 fleet       age_length_conversion_matrix vector
       11 Fleet               1 fleet       age_length_conversion_matrix vector
       12 Fleet               1 fleet       age_length_conversion_matrix vector
       13 Fleet               1 fleet       age_length_conversion_matrix vector
       14 Fleet               1 fleet       age_length_conversion_matrix vector
       15 Fleet               1 fleet       age_length_conversion_matrix vector
       16 Fleet               1 fleet       age_length_conversion_matrix vector
       17 Fleet               1 fleet       age_length_conversion_matrix vector
       18 Fleet               1 fleet       age_length_conversion_matrix vector
       19 Fleet               1 fleet       age_length_conversion_matrix vector
       20 Fleet               1 fleet       age_length_conversion_matrix vector
       21 Fleet               1 fleet       age_length_conversion_matrix vector
       22 Fleet               1 fleet       age_length_conversion_matrix vector
       23 Fleet               1 fleet       age_length_conversion_matrix vector
       24 Fleet               1 fleet       age_length_conversion_matrix vector
       25 Fleet               1 fleet       age_length_conversion_matrix vector
       26 Fleet               1 fleet       age_length_conversion_matrix vector
       27 Fleet               1 fleet       age_length_conversion_matrix vector
       28 Fleet               1 fleet       age_length_conversion_matrix vector
       29 Fleet               1 fleet       age_length_conversion_matrix vector
       30 Fleet               1 fleet       age_length_conversion_matrix vector
       31 Fleet               1 fleet       age_length_conversion_matrix vector
       32 Fleet               1 fleet       age_length_conversion_matrix vector
       33 Fleet               1 fleet       age_length_conversion_matrix vector
       34 Fleet               1 fleet       age_length_conversion_matrix vector
       35 Fleet               1 fleet       age_length_conversion_matrix vector
       36 Fleet               1 fleet       age_length_conversion_matrix vector
       37 Fleet               1 fleet       age_length_conversion_matrix vector
       38 Fleet               1 fleet       age_length_conversion_matrix vector
       39 Fleet               1 fleet       age_length_conversion_matrix vector
       40 Fleet               1 fleet       age_length_conversion_matrix vector
       41 Fleet               1 fleet       age_length_conversion_matrix vector
       42 Fleet               1 fleet       age_length_conversion_matrix vector
       43 Fleet               1 fleet       age_length_conversion_matrix vector
       44 Fleet               1 fleet       age_length_conversion_matrix vector
       45 Fleet               1 fleet       age_length_conversion_matrix vector
       46 Fleet               1 fleet       age_length_conversion_matrix vector
       47 Fleet               1 fleet       age_length_conversion_matrix vector
       48 Fleet               1 fleet       age_length_conversion_matrix vector
       49 Fleet               1 fleet       age_length_conversion_matrix vector
       50 Fleet               1 fleet       age_length_conversion_matrix vector
       51 Fleet               1 fleet       age_length_conversion_matrix vector
       52 Fleet               1 fleet       age_length_conversion_matrix vector
       53 Fleet               1 fleet       age_length_conversion_matrix vector
       54 Fleet               1 fleet       age_length_conversion_matrix vector
       55 Fleet               1 fleet       age_length_conversion_matrix vector
       56 Fleet               1 fleet       age_length_conversion_matrix vector
       57 Fleet               1 fleet       age_length_conversion_matrix vector
       58 Fleet               1 fleet       age_length_conversion_matrix vector
       59 Fleet               1 fleet       age_length_conversion_matrix vector
       60 Fleet               1 fleet       age_length_conversion_matrix vector
       61 Fleet               1 fleet       age_length_conversion_matrix vector
       62 Fleet               1 fleet       age_length_conversion_matrix vector
       63 Fleet               1 fleet       age_length_conversion_matrix vector
       64 Fleet               1 fleet       age_length_conversion_matrix vector
       65 Fleet               1 fleet       age_length_conversion_matrix vector
       66 Fleet               1 fleet       age_length_conversion_matrix vector
       67 Fleet               1 fleet       age_length_conversion_matrix vector
       68 Fleet               1 fleet       age_length_conversion_matrix vector
       69 Fleet               1 fleet       age_length_conversion_matrix vector
       70 Fleet               1 fleet       age_length_conversion_matrix vector
       71 Fleet               1 fleet       age_length_conversion_matrix vector
       72 Fleet               1 fleet       age_length_conversion_matrix vector
       73 Fleet               1 fleet       age_length_conversion_matrix vector
       74 Fleet               1 fleet       age_length_conversion_matrix vector
       75 Fleet               1 fleet       age_length_conversion_matrix vector
       76 Fleet               1 fleet       age_length_conversion_matrix vector
       77 Fleet               1 fleet       age_length_conversion_matrix vector
       78 Fleet               1 fleet       age_length_conversion_matrix vector
       79 Fleet               1 fleet       age_length_conversion_matrix vector
       80 Fleet               1 fleet       age_length_conversion_matrix vector
       81 Fleet               1 fleet       age_length_conversion_matrix vector
       82 Fleet               1 fleet       age_length_conversion_matrix vector
       83 Fleet               1 fleet       age_length_conversion_matrix vector
       84 Fleet               1 fleet       age_length_conversion_matrix vector
       85 Fleet               1 fleet       age_length_conversion_matrix vector
       86 Fleet               1 fleet       age_length_conversion_matrix vector
       87 Fleet               1 fleet       age_length_conversion_matrix vector
       88 Fleet               1 fleet       age_length_conversion_matrix vector
       89 Fleet               1 fleet       age_length_conversion_matrix vector
       90 Fleet               1 fleet       age_length_conversion_matrix vector
       91 Fleet               1 fleet       age_length_conversion_matrix vector
       92 Fleet               1 fleet       age_length_conversion_matrix vector
       93 Fleet               1 fleet       age_length_conversion_matrix vector
       94 Fleet               1 fleet       age_length_conversion_matrix vector
       95 Fleet               1 fleet       age_length_conversion_matrix vector
       96 Fleet               1 fleet       age_length_conversion_matrix vector
       97 Fleet               1 fleet       age_length_conversion_matrix vector
       98 Fleet               1 fleet       age_length_conversion_matrix vector
       99 Fleet               1 fleet       age_length_conversion_matrix vector
      100 Fleet               1 fleet       age_length_conversion_matrix vector
      101 Fleet               1 fleet       age_length_conversion_matrix vector
      102 Fleet               1 fleet       age_length_conversion_matrix vector
      103 Fleet               1 fleet       age_length_conversion_matrix vector
      104 Fleet               1 fleet       age_length_conversion_matrix vector
      105 Fleet               1 fleet       age_length_conversion_matrix vector
      106 Fleet               1 fleet       age_length_conversion_matrix vector
      107 Fleet               1 fleet       age_length_conversion_matrix vector
      108 Fleet               1 fleet       age_length_conversion_matrix vector
      109 Fleet               1 fleet       age_length_conversion_matrix vector
      110 Fleet               1 fleet       age_length_conversion_matrix vector
      111 Fleet               1 fleet       age_length_conversion_matrix vector
      112 Fleet               1 fleet       age_length_conversion_matrix vector
      113 Fleet               1 fleet       age_length_conversion_matrix vector
      114 Fleet               1 fleet       age_length_conversion_matrix vector
      115 Fleet               1 fleet       age_length_conversion_matrix vector
      116 Fleet               1 fleet       age_length_conversion_matrix vector
      117 Fleet               1 fleet       age_length_conversion_matrix vector
      118 Fleet               1 fleet       age_length_conversion_matrix vector
      119 Fleet               1 fleet       age_length_conversion_matrix vector
      120 Fleet               1 fleet       age_length_conversion_matrix vector
      121 Fleet               1 fleet       age_length_conversion_matrix vector
      122 Fleet               1 fleet       age_length_conversion_matrix vector
      123 Fleet               1 fleet       age_length_conversion_matrix vector
      124 Fleet               1 fleet       age_length_conversion_matrix vector
      125 Fleet               1 fleet       age_length_conversion_matrix vector
      126 Fleet               1 fleet       age_length_conversion_matrix vector
      127 Fleet               1 fleet       age_length_conversion_matrix vector
      128 Fleet               1 fleet       age_length_conversion_matrix vector
      129 Fleet               1 fleet       age_length_conversion_matrix vector
      130 Fleet               1 fleet       age_length_conversion_matrix vector
      131 Fleet               1 fleet       age_length_conversion_matrix vector
      132 Fleet               1 fleet       age_length_conversion_matrix vector
      133 Fleet               1 fleet       age_length_conversion_matrix vector
      134 Fleet               1 fleet       age_length_conversion_matrix vector
      135 Fleet               1 fleet       age_length_conversion_matrix vector
      136 Fleet               1 fleet       age_length_conversion_matrix vector
      137 Fleet               1 fleet       age_length_conversion_matrix vector
      138 Fleet               1 fleet       age_length_conversion_matrix vector
      139 Fleet               1 fleet       age_length_conversion_matrix vector
      140 Fleet               1 fleet       age_length_conversion_matrix vector
      141 Fleet               1 fleet       age_length_conversion_matrix vector
      142 Fleet               1 fleet       age_length_conversion_matrix vector
      143 Fleet               1 fleet       age_length_conversion_matrix vector
      144 Fleet               1 fleet       age_length_conversion_matrix vector
      145 Fleet               1 fleet       age_length_conversion_matrix vector
      146 Fleet               1 fleet       age_length_conversion_matrix vector
      147 Fleet               1 fleet       age_length_conversion_matrix vector
      148 Fleet               1 fleet       age_length_conversion_matrix vector
      149 Fleet               1 fleet       age_length_conversion_matrix vector
      150 Fleet               1 fleet       age_length_conversion_matrix vector
      151 Fleet               1 fleet       age_length_conversion_matrix vector
      152 Fleet               1 fleet       age_length_conversion_matrix vector
      153 Fleet               1 fleet       age_length_conversion_matrix vector
      154 Fleet               1 fleet       age_length_conversion_matrix vector
      155 Fleet               1 fleet       age_length_conversion_matrix vector
      156 Fleet               1 fleet       age_length_conversion_matrix vector
      157 Fleet               1 fleet       age_length_conversion_matrix vector
      158 Fleet               1 fleet       age_length_conversion_matrix vector
      159 Fleet               1 fleet       age_length_conversion_matrix vector
      160 Fleet               1 fleet       age_length_conversion_matrix vector
      161 Fleet               1 fleet       age_length_conversion_matrix vector
      162 Fleet               1 fleet       age_length_conversion_matrix vector
      163 Fleet               1 fleet       age_length_conversion_matrix vector
      164 Fleet               1 fleet       age_length_conversion_matrix vector
      165 Fleet               1 fleet       age_length_conversion_matrix vector
      166 Fleet               1 fleet       age_length_conversion_matrix vector
      167 Fleet               1 fleet       age_length_conversion_matrix vector
      168 Fleet               1 fleet       age_length_conversion_matrix vector
      169 Fleet               1 fleet       age_length_conversion_matrix vector
      170 Fleet               1 fleet       age_length_conversion_matrix vector
      171 Fleet               1 fleet       age_length_conversion_matrix vector
      172 Fleet               1 fleet       age_length_conversion_matrix vector
      173 Fleet               1 fleet       age_length_conversion_matrix vector
      174 Fleet               1 fleet       age_length_conversion_matrix vector
      175 Fleet               1 fleet       age_length_conversion_matrix vector
      176 Fleet               1 fleet       age_length_conversion_matrix vector
      177 Fleet               1 fleet       age_length_conversion_matrix vector
      178 Fleet               1 fleet       age_length_conversion_matrix vector
      179 Fleet               1 fleet       age_length_conversion_matrix vector
      180 Fleet               1 fleet       age_length_conversion_matrix vector
      181 Fleet               1 fleet       age_length_conversion_matrix vector
      182 Fleet               1 fleet       age_length_conversion_matrix vector
      183 Fleet               1 fleet       age_length_conversion_matrix vector
      184 Fleet               1 fleet       age_length_conversion_matrix vector
      185 Fleet               1 fleet       age_length_conversion_matrix vector
      186 Fleet               1 fleet       age_length_conversion_matrix vector
      187 Fleet               1 fleet       age_length_conversion_matrix vector
      188 Fleet               1 fleet       age_length_conversion_matrix vector
      189 Fleet               1 fleet       age_length_conversion_matrix vector
      190 Fleet               1 fleet       age_length_conversion_matrix vector
      191 Fleet               1 fleet       age_length_conversion_matrix vector
      192 Fleet               1 fleet       age_length_conversion_matrix vector
      193 Fleet               1 fleet       age_length_conversion_matrix vector
      194 Fleet               1 fleet       age_length_conversion_matrix vector
      195 Fleet               1 fleet       age_length_conversion_matrix vector
      196 Fleet               1 fleet       age_length_conversion_matrix vector
      197 Fleet               1 fleet       age_length_conversion_matrix vector
      198 Fleet               1 fleet       age_length_conversion_matrix vector
      199 Fleet               1 fleet       age_length_conversion_matrix vector
      200 Fleet               1 fleet       age_length_conversion_matrix vector
      201 Fleet               1 fleet       age_length_conversion_matrix vector
      202 Fleet               1 fleet       age_length_conversion_matrix vector
      203 Fleet               1 fleet       age_length_conversion_matrix vector
      204 Fleet               1 fleet       age_length_conversion_matrix vector
      205 Fleet               1 fleet       age_length_conversion_matrix vector
      206 Fleet               1 fleet       age_length_conversion_matrix vector
      207 Fleet               1 fleet       age_length_conversion_matrix vector
      208 Fleet               1 fleet       age_length_conversion_matrix vector
      209 Fleet               1 fleet       age_length_conversion_matrix vector
      210 Fleet               1 fleet       age_length_conversion_matrix vector
      211 Fleet               1 fleet       age_length_conversion_matrix vector
      212 Fleet               1 fleet       age_length_conversion_matrix vector
      213 Fleet               1 fleet       age_length_conversion_matrix vector
      214 Fleet               1 fleet       age_length_conversion_matrix vector
      215 Fleet               1 fleet       age_length_conversion_matrix vector
      216 Fleet               1 fleet       age_length_conversion_matrix vector
      217 Fleet               1 fleet       age_length_conversion_matrix vector
      218 Fleet               1 fleet       age_length_conversion_matrix vector
      219 Fleet               1 fleet       age_length_conversion_matrix vector
      220 Fleet               1 fleet       age_length_conversion_matrix vector
      221 Fleet               1 fleet       age_length_conversion_matrix vector
      222 Fleet               1 fleet       age_length_conversion_matrix vector
      223 Fleet               1 fleet       age_length_conversion_matrix vector
      224 Fleet               1 fleet       age_length_conversion_matrix vector
      225 Fleet               1 fleet       age_length_conversion_matrix vector
      226 Fleet               1 fleet       age_length_conversion_matrix vector
      227 Fleet               1 fleet       age_length_conversion_matrix vector
      228 Fleet               1 fleet       age_length_conversion_matrix vector
      229 Fleet               1 fleet       age_length_conversion_matrix vector
      230 Fleet               1 fleet       age_length_conversion_matrix vector
      231 Fleet               1 fleet       age_length_conversion_matrix vector
      232 Fleet               1 fleet       age_length_conversion_matrix vector
      233 Fleet               1 fleet       age_length_conversion_matrix vector
      234 Fleet               1 fleet       age_length_conversion_matrix vector
      235 Fleet               1 fleet       age_length_conversion_matrix vector
      236 Fleet               1 fleet       age_length_conversion_matrix vector
      237 Fleet               1 fleet       age_length_conversion_matrix vector
      238 Fleet               1 fleet       age_length_conversion_matrix vector
      239 Fleet               1 fleet       age_length_conversion_matrix vector
      240 Fleet               1 fleet       age_length_conversion_matrix vector
      241 Fleet               1 fleet       age_length_conversion_matrix vector
      242 Fleet               1 fleet       age_length_conversion_matrix vector
      243 Fleet               1 fleet       age_length_conversion_matrix vector
      244 Fleet               1 fleet       age_length_conversion_matrix vector
      245 Fleet               1 fleet       age_length_conversion_matrix vector
      246 Fleet               1 fleet       age_length_conversion_matrix vector
      247 Fleet               1 fleet       age_length_conversion_matrix vector
      248 Fleet               1 fleet       age_length_conversion_matrix vector
      249 Fleet               1 fleet       age_length_conversion_matrix vector
      250 Fleet               1 fleet       age_length_conversion_matrix vector
      251 Fleet               1 fleet       age_length_conversion_matrix vector
      252 Fleet               1 fleet       age_length_conversion_matrix vector
      253 Fleet               1 fleet       age_length_conversion_matrix vector
      254 Fleet               1 fleet       age_length_conversion_matrix vector
      255 Fleet               1 fleet       age_length_conversion_matrix vector
      256 Fleet               1 fleet       age_length_conversion_matrix vector
      257 Fleet               1 fleet       age_length_conversion_matrix vector
      258 Fleet               1 fleet       age_length_conversion_matrix vector
      259 Fleet               1 fleet       age_length_conversion_matrix vector
      260 Fleet               1 fleet       age_length_conversion_matrix vector
      261 Fleet               1 fleet       age_length_conversion_matrix vector
      262 Fleet               1 fleet       age_length_conversion_matrix vector
      263 Fleet               1 fleet       age_length_conversion_matrix vector
      264 Fleet               1 fleet       age_length_conversion_matrix vector
      265 Fleet               1 fleet       age_length_conversion_matrix vector
      266 Fleet               1 fleet       age_length_conversion_matrix vector
      267 Fleet               1 fleet       age_length_conversion_matrix vector
      268 Fleet               1 fleet       age_length_conversion_matrix vector
      269 Fleet               1 fleet       age_length_conversion_matrix vector
      270 Fleet               1 fleet       age_length_conversion_matrix vector
      271 Fleet               1 fleet       age_length_conversion_matrix vector
      272 Fleet               1 fleet       age_length_conversion_matrix vector
      273 Fleet               1 fleet       age_length_conversion_matrix vector
      274 Fleet               1 fleet       age_length_conversion_matrix vector
      275 Fleet               1 fleet       age_length_conversion_matrix vector
      276 Fleet               1 fleet       age_length_conversion_matrix vector
      277 Fleet               1 fleet       age_length_conversion_matrix vector
      278 Fleet               1 fleet       age_length_conversion_matrix vector
      279 Fleet               1 fleet       age_length_conversion_matrix vector
      280 Fleet               1 fleet       age_length_conversion_matrix vector
      281 Fleet               1 fleet       log_Fmort                    vector
      282 Fleet               1 fleet       log_Fmort                    vector
      283 Fleet               1 fleet       log_Fmort                    vector
      284 Fleet               1 fleet       log_Fmort                    vector
      285 Fleet               1 fleet       log_Fmort                    vector
      286 Fleet               1 fleet       log_Fmort                    vector
      287 Fleet               1 fleet       log_Fmort                    vector
      288 Fleet               1 fleet       log_Fmort                    vector
      289 Fleet               1 fleet       log_Fmort                    vector
      290 Fleet               1 fleet       log_Fmort                    vector
      291 Fleet               1 fleet       log_Fmort                    vector
      292 Fleet               1 fleet       log_Fmort                    vector
      293 Fleet               1 fleet       log_Fmort                    vector
      294 Fleet               1 fleet       log_Fmort                    vector
      295 Fleet               1 fleet       log_Fmort                    vector
      296 Fleet               1 fleet       log_Fmort                    vector
      297 Fleet               1 fleet       log_Fmort                    vector
      298 Fleet               1 fleet       log_Fmort                    vector
      299 Fleet               1 fleet       log_Fmort                    vector
      300 Fleet               1 fleet       log_Fmort                    vector
      301 Fleet               1 fleet       log_Fmort                    vector
      302 Fleet               1 fleet       log_Fmort                    vector
      303 Fleet               1 fleet       log_Fmort                    vector
      304 Fleet               1 fleet       log_Fmort                    vector
      305 Fleet               1 fleet       log_Fmort                    vector
      306 Fleet               1 fleet       log_Fmort                    vector
      307 Fleet               1 fleet       log_Fmort                    vector
      308 Fleet               1 fleet       log_Fmort                    vector
      309 Fleet               1 fleet       log_Fmort                    vector
      310 selectivity         2 Logistic    inflection_point             vector
      311 selectivity         2 Logistic    slope                        vector
      312 Fleet               2 fleet       log_q                        vector
      313 Fleet               2 fleet       log_Fmort                    vector
      314 Fleet               2 fleet       age_length_conversion_matrix vector
      315 Fleet               2 fleet       age_length_conversion_matrix vector
      316 Fleet               2 fleet       age_length_conversion_matrix vector
      317 Fleet               2 fleet       age_length_conversion_matrix vector
      318 Fleet               2 fleet       age_length_conversion_matrix vector
      319 Fleet               2 fleet       age_length_conversion_matrix vector
      320 Fleet               2 fleet       age_length_conversion_matrix vector
          type_id parameter_id fleet_name   age length  time   initial log_like
            <int>        <int> <chr>      <dbl>  <dbl> <int>     <dbl>    <dbl>
        1       1            1 <NA>          NA     NA    NA  2   e+ 0       NA
        2       2            2 <NA>          NA     NA    NA  1   e+ 0       NA
        3       3            3 <NA>          NA     NA    NA  0              NA
        4       4            4 <NA>          NA     NA    NA -4.66e+ 0       NA
        5       8            8 <NA>          NA     NA    NA  1.26e-16       NA
        6       8            9 <NA>          NA     NA    NA  8.39e-11       NA
        7       8           10 <NA>          NA     NA    NA  2.30e- 6       NA
        8       8           11 <NA>          NA     NA    NA  2.74e- 3       NA
        9       8           12 <NA>          NA     NA    NA  1.63e- 1       NA
       10       8           13 <NA>          NA     NA    NA  6.32e- 1       NA
       11       8           14 <NA>          NA     NA    NA  1.98e- 1       NA
       12       8           15 <NA>          NA     NA    NA  4.13e- 3       NA
       13       8           16 <NA>          NA     NA    NA  4.36e- 6       NA
       14       8           17 <NA>          NA     NA    NA  2.02e-10       NA
       15       8           18 <NA>          NA     NA    NA  3.33e-16       NA
       16       8           19 <NA>          NA     NA    NA  0              NA
       17       8           20 <NA>          NA     NA    NA  0              NA
       18       8           21 <NA>          NA     NA    NA  0              NA
       19       8           22 <NA>          NA     NA    NA  0              NA
       20       8           23 <NA>          NA     NA    NA  0              NA
       21       8           24 <NA>          NA     NA    NA  0              NA
       22       8           25 <NA>          NA     NA    NA  0              NA
       23       8           26 <NA>          NA     NA    NA  0              NA
       24       8           27 <NA>          NA     NA    NA  0              NA
       25       8           28 <NA>          NA     NA    NA  0              NA
       26       8           29 <NA>          NA     NA    NA  0              NA
       27       8           30 <NA>          NA     NA    NA  0              NA
       28       8           31 <NA>          NA     NA    NA  3.27e-18       NA
       29       8           32 <NA>          NA     NA    NA  2.15e-13       NA
       30       8           33 <NA>          NA     NA    NA  2.20e- 9       NA
       31       8           34 <NA>          NA     NA    NA  3.54e- 6       NA
       32       8           35 <NA>          NA     NA    NA  9.19e- 4       NA
       33       8           36 <NA>          NA     NA    NA  4.03e- 2       NA
       34       8           37 <NA>          NA     NA    NA  3.18e- 1       NA
       35       8           38 <NA>          NA     NA    NA  4.86e- 1       NA
       36       8           39 <NA>          NA     NA    NA  1.46e- 1       NA
       37       8           40 <NA>          NA     NA    NA  8.23e- 3       NA
       38       8           41 <NA>          NA     NA    NA  8.08e- 5       NA
       39       8           42 <NA>          NA     NA    NA  1.31e- 7       NA
       40       8           43 <NA>          NA     NA    NA  3.37e-11       NA
       41       8           44 <NA>          NA     NA    NA  1.33e-15       NA
       42       8           45 <NA>          NA     NA    NA  0              NA
       43       8           46 <NA>          NA     NA    NA  0              NA
       44       8           47 <NA>          NA     NA    NA  0              NA
       45       8           48 <NA>          NA     NA    NA  0              NA
       46       8           49 <NA>          NA     NA    NA  0              NA
       47       8           50 <NA>          NA     NA    NA  0              NA
       48       8           51 <NA>          NA     NA    NA  0              NA
       49       8           52 <NA>          NA     NA    NA  0              NA
       50       8           53 <NA>          NA     NA    NA  0              NA
       51       8           54 <NA>          NA     NA    NA  4.35e-19       NA
       52       8           55 <NA>          NA     NA    NA  6.73e-15       NA
       53       8           56 <NA>          NA     NA    NA  2.84e-11       NA
       54       8           57 <NA>          NA     NA    NA  3.28e- 8       NA
       55       8           58 <NA>          NA     NA    NA  1.05e- 5       NA
       56       8           59 <NA>          NA     NA    NA  9.44e- 4       NA
       57       8           60 <NA>          NA     NA    NA  2.44e- 2       NA
       58       8           61 <NA>          NA     NA    NA  1.85e- 1       NA
       59       8           62 <NA>          NA     NA    NA  4.24e- 1       NA
       60       8           63 <NA>          NA     NA    NA  2.98e- 1       NA
       61       8           64 <NA>          NA     NA    NA  6.36e- 2       NA
       62       8           65 <NA>          NA     NA    NA  4.04e- 3       NA
       63       8           66 <NA>          NA     NA    NA  7.43e- 5       NA
       64       8           67 <NA>          NA     NA    NA  3.87e- 7       NA
       65       8           68 <NA>          NA     NA    NA  5.62e-10       NA
       66       8           69 <NA>          NA     NA    NA  2.24e-13       NA
       67       8           70 <NA>          NA     NA    NA  0              NA
       68       8           71 <NA>          NA     NA    NA  0              NA
       69       8           72 <NA>          NA     NA    NA  0              NA
       70       8           73 <NA>          NA     NA    NA  0              NA
       71       8           74 <NA>          NA     NA    NA  0              NA
       72       8           75 <NA>          NA     NA    NA  0              NA
       73       8           76 <NA>          NA     NA    NA  0              NA
       74       8           77 <NA>          NA     NA    NA  1.23e-19       NA
       75       8           78 <NA>          NA     NA    NA  7.29e-16       NA
       76       8           79 <NA>          NA     NA    NA  1.58e-12       NA
       77       8           80 <NA>          NA     NA    NA  1.25e- 9       NA
       78       8           81 <NA>          NA     NA    NA  3.68e- 7       NA
       79       8           82 <NA>          NA     NA    NA  4.02e- 5       NA
       80       8           83 <NA>          NA     NA    NA  1.65e- 3       NA
       81       8           84 <NA>          NA     NA    NA  2.56e- 2       NA
       82       8           85 <NA>          NA     NA    NA  1.54e- 1       NA
       83       8           86 <NA>          NA     NA    NA  3.58e- 1       NA
       84       8           87 <NA>          NA     NA    NA  3.27e- 1       NA
       85       8           88 <NA>          NA     NA    NA  1.17e- 1       NA
       86       8           89 <NA>          NA     NA    NA  1.62e- 2       NA
       87       8           90 <NA>          NA     NA    NA  8.65e- 4       NA
       88       8           91 <NA>          NA     NA    NA  1.75e- 5       NA
       89       8           92 <NA>          NA     NA    NA  1.32e- 7       NA
       90       8           93 <NA>          NA     NA    NA  3.71e-10       NA
       91       8           94 <NA>          NA     NA    NA  3.85e-13       NA
       92       8           95 <NA>          NA     NA    NA  1.11e-16       NA
       93       8           96 <NA>          NA     NA    NA  0              NA
       94       8           97 <NA>          NA     NA    NA  0              NA
       95       8           98 <NA>          NA     NA    NA  0              NA
       96       8           99 <NA>          NA     NA    NA  0              NA
       97       8          100 <NA>          NA     NA    NA  5.27e-20       NA
       98       8          101 <NA>          NA     NA    NA  1.59e-16       NA
       99       8          102 <NA>          NA     NA    NA  2.09e-13       NA
      100       8          103 <NA>          NA     NA    NA  1.20e-10       NA
      101       8          104 <NA>          NA     NA    NA  3.04e- 8       NA
      102       8          105 <NA>          NA     NA    NA  3.38e- 6       NA
      103       8          106 <NA>          NA     NA    NA  1.67e- 4       NA
      104       8          107 <NA>          NA     NA    NA  3.67e- 3       NA
      105       8          108 <NA>          NA     NA    NA  3.63e- 2       NA
      106       8          109 <NA>          NA     NA    NA  1.63e- 1       NA
      107       8          110 <NA>          NA     NA    NA  3.31e- 1       NA
      108       8          111 <NA>          NA     NA    NA  3.08e- 1       NA
      109       8          112 <NA>          NA     NA    NA  1.31e- 1       NA
      110       8          113 <NA>          NA     NA    NA  2.52e- 2       NA
      111       8          114 <NA>          NA     NA    NA  2.20e- 3       NA
      112       8          115 <NA>          NA     NA    NA  8.62e- 5       NA
      113       8          116 <NA>          NA     NA    NA  1.50e- 6       NA
      114       8          117 <NA>          NA     NA    NA  1.16e- 8       NA
      115       8          118 <NA>          NA     NA    NA  3.94e-11       NA
      116       8          119 <NA>          NA     NA    NA  5.88e-14       NA
      117       8          120 <NA>          NA     NA    NA  0              NA
      118       8          121 <NA>          NA     NA    NA  0              NA
      119       8          122 <NA>          NA     NA    NA  0              NA
      120       8          123 <NA>          NA     NA    NA  2.88e-20       NA
      121       8          124 <NA>          NA     NA    NA  5.32e-17       NA
      122       8          125 <NA>          NA     NA    NA  4.81e-14       NA
      123       8          126 <NA>          NA     NA    NA  2.13e-11       NA
      124       8          127 <NA>          NA     NA    NA  4.62e- 9       NA
      125       8          128 <NA>          NA     NA    NA  4.94e- 7       NA
      126       8          129 <NA>          NA     NA    NA  2.61e- 5       NA
      127       8          130 <NA>          NA     NA    NA  6.86e- 4       NA
      128       8          131 <NA>          NA     NA    NA  8.98e- 3       NA
      129       8          132 <NA>          NA     NA    NA  5.89e- 2       NA
      130       8          133 <NA>          NA     NA    NA  1.94e- 1       NA
      131       8          134 <NA>          NA     NA    NA  3.23e- 1       NA
      132       8          135 <NA>          NA     NA    NA  2.72e- 1       NA
      133       8          136 <NA>          NA     NA    NA  1.15e- 1       NA
      134       8          137 <NA>          NA     NA    NA  2.47e- 2       NA
      135       8          138 <NA>          NA     NA    NA  2.66e- 3       NA
      136       8          139 <NA>          NA     NA    NA  1.43e- 4       NA
      137       8          140 <NA>          NA     NA    NA  3.81e- 6       NA
      138       8          141 <NA>          NA     NA    NA  5.04e- 8       NA
      139       8          142 <NA>          NA     NA    NA  3.29e-10       NA
      140       8          143 <NA>          NA     NA    NA  1.06e-12       NA
      141       8          144 <NA>          NA     NA    NA  1.67e-15       NA
      142       8          145 <NA>          NA     NA    NA  0              NA
      143       8          146 <NA>          NA     NA    NA  1.85e-20       NA
      144       8          147 <NA>          NA     NA    NA  2.36e-17       NA
      145       8          148 <NA>          NA     NA    NA  1.60e-14       NA
      146       8          149 <NA>          NA     NA    NA  5.73e-12       NA
      147       8          150 <NA>          NA     NA    NA  1.09e- 9       NA
      148       8          151 <NA>          NA     NA    NA  1.10e- 7       NA
      149       8          152 <NA>          NA     NA    NA  5.93e- 6       NA
      150       8          153 <NA>          NA     NA    NA  1.71e- 4       NA
      151       8          154 <NA>          NA     NA    NA  2.63e- 3       NA
      152       8          155 <NA>          NA     NA    NA  2.18e- 2       NA
      153       8          156 <NA>          NA     NA    NA  9.79e- 2       NA
      154       8          157 <NA>          NA     NA    NA  2.37e- 1       NA
      155       8          158 <NA>          NA     NA    NA  3.12e- 1       NA
      156       8          159 <NA>          NA     NA    NA  2.22e- 1       NA
      157       8          160 <NA>          NA     NA    NA  8.59e- 2       NA
      158       8          161 <NA>          NA     NA    NA  1.80e- 2       NA
      159       8          162 <NA>          NA     NA    NA  2.03e- 3       NA
      160       8          163 <NA>          NA     NA    NA  1.23e- 4       NA
      161       8          164 <NA>          NA     NA    NA  4.00e- 6       NA
      162       8          165 <NA>          NA     NA    NA  6.96e- 8       NA
      163       8          166 <NA>          NA     NA    NA  6.44e-10       NA
      164       8          167 <NA>          NA     NA    NA  3.17e-12       NA
      165       8          168 <NA>          NA     NA    NA  8.33e-15       NA
      166       8          169 <NA>          NA     NA    NA  1.32e-20       NA
      167       8          170 <NA>          NA     NA    NA  1.28e-17       NA
      168       8          171 <NA>          NA     NA    NA  6.90e-15       NA
      169       8          172 <NA>          NA     NA    NA  2.09e-12       NA
      170       8          173 <NA>          NA     NA    NA  3.54e-10       NA
      171       8          174 <NA>          NA     NA    NA  3.37e- 8       NA
      172       8          175 <NA>          NA     NA    NA  1.81e- 6       NA
      173       8          176 <NA>          NA     NA    NA  5.45e- 5       NA
      174       8          177 <NA>          NA     NA    NA  9.31e- 4       NA
      175       8          178 <NA>          NA     NA    NA  9.00e- 3       NA
      176       8          179 <NA>          NA     NA    NA  4.95e- 2       NA
      177       8          180 <NA>          NA     NA    NA  1.55e- 1       NA
      178       8          181 <NA>          NA     NA    NA  2.76e- 1       NA
      179       8          182 <NA>          NA     NA    NA  2.81e- 1       NA
      180       8          183 <NA>          NA     NA    NA  1.63e- 1       NA
      181       8          184 <NA>          NA     NA    NA  5.42e- 2       NA
      182       8          185 <NA>          NA     NA    NA  1.02e- 2       NA
      183       8          186 <NA>          NA     NA    NA  1.10e- 3       NA
      184       8          187 <NA>          NA     NA    NA  6.66e- 5       NA
      185       8          188 <NA>          NA     NA    NA  2.29e- 6       NA
      186       8          189 <NA>          NA     NA    NA  4.44e- 8       NA
      187       8          190 <NA>          NA     NA    NA  4.83e-10       NA
      188       8          191 <NA>          NA     NA    NA  2.97e-12       NA
      189       8          192 <NA>          NA     NA    NA  1.02e-20       NA
      190       8          193 <NA>          NA     NA    NA  7.91e-18       NA
      191       8          194 <NA>          NA     NA    NA  3.58e-15       NA
      192       8          195 <NA>          NA     NA    NA  9.46e-13       NA
      193       8          196 <NA>          NA     NA    NA  1.46e-10       NA
      194       8          197 <NA>          NA     NA    NA  1.31e- 8       NA
      195       8          198 <NA>          NA     NA    NA  6.94e- 7       NA
      196       8          199 <NA>          NA     NA    NA  2.15e- 5       NA
      197       8          200 <NA>          NA     NA    NA  3.91e- 4       NA
      198       8          201 <NA>          NA     NA    NA  4.19e- 3       NA
      199       8          202 <NA>          NA     NA    NA  2.65e- 2       NA
      200       8          203 <NA>          NA     NA    NA  9.92e- 2       NA
      201       8          204 <NA>          NA     NA    NA  2.20e- 1       NA
      202       8          205 <NA>          NA     NA    NA  2.88e- 1       NA
      203       8          206 <NA>          NA     NA    NA  2.25e- 1       NA
      204       8          207 <NA>          NA     NA    NA  1.04e- 1       NA
      205       8          208 <NA>          NA     NA    NA  2.83e- 2       NA
      206       8          209 <NA>          NA     NA    NA  4.58e- 3       NA
      207       8          210 <NA>          NA     NA    NA  4.37e- 4       NA
      208       8          211 <NA>          NA     NA    NA  2.46e- 5       NA
      209       8          212 <NA>          NA     NA    NA  8.11e- 7       NA
      210       8          213 <NA>          NA     NA    NA  1.57e- 8       NA
      211       8          214 <NA>          NA     NA    NA  1.80e-10       NA
      212       8          215 <NA>          NA     NA    NA  8.31e-21       NA
      213       8          216 <NA>          NA     NA    NA  5.43e-18       NA
      214       8          217 <NA>          NA     NA    NA  2.14e-15       NA
      215       8          218 <NA>          NA     NA    NA  5.06e-13       NA
      216       8          219 <NA>          NA     NA    NA  7.20e-11       NA
      217       8          220 <NA>          NA     NA    NA  6.18e- 9       NA
      218       8          221 <NA>          NA     NA    NA  3.20e- 7       NA
      219       8          222 <NA>          NA     NA    NA  1.00e- 5       NA
      220       8          223 <NA>          NA     NA    NA  1.91e- 4       NA
      221       8          224 <NA>          NA     NA    NA  2.19e- 3       NA
      222       8          225 <NA>          NA     NA    NA  1.53e- 2       NA
      223       8          226 <NA>          NA     NA    NA  6.53e- 2       NA
      224       8          227 <NA>          NA     NA    NA  1.69e- 1       NA
      225       8          228 <NA>          NA     NA    NA  2.68e- 1       NA
      226       8          229 <NA>          NA     NA    NA  2.59e- 1       NA
      227       8          230 <NA>          NA     NA    NA  1.53e- 1       NA
      228       8          231 <NA>          NA     NA    NA  5.48e- 2       NA
      229       8          232 <NA>          NA     NA    NA  1.20e- 2       NA
      230       8          233 <NA>          NA     NA    NA  1.60e- 3       NA
      231       8          234 <NA>          NA     NA    NA  1.30e- 4       NA
      232       8          235 <NA>          NA     NA    NA  6.37e- 6       NA
      233       8          236 <NA>          NA     NA    NA  1.89e- 7       NA
      234       8          237 <NA>          NA     NA    NA  3.44e- 9       NA
      235       8          238 <NA>          NA     NA    NA  7.07e-21       NA
      236       8          239 <NA>          NA     NA    NA  4.03e-18       NA
      237       8          240 <NA>          NA     NA    NA  1.42e-15       NA
      238       8          241 <NA>          NA     NA    NA  3.06e-13       NA
      239       8          242 <NA>          NA     NA    NA  4.09e-11       NA
      240       8          243 <NA>          NA     NA    NA  3.36e- 9       NA
      241       8          244 <NA>          NA     NA    NA  1.71e- 7       NA
      242       8          245 <NA>          NA     NA    NA  5.39e- 6       NA
      243       8          246 <NA>          NA     NA    NA  1.05e- 4       NA
      244       8          247 <NA>          NA     NA    NA  1.27e- 3       NA
      245       8          248 <NA>          NA     NA    NA  9.57e- 3       NA
      246       8          249 <NA>          NA     NA    NA  4.48e- 2       NA
      247       8          250 <NA>          NA     NA    NA  1.31e- 1       NA
      248       8          251 <NA>          NA     NA    NA  2.38e- 1       NA
      249       8          252 <NA>          NA     NA    NA  2.71e- 1       NA
      250       8          253 <NA>          NA     NA    NA  1.92e- 1       NA
      251       8          254 <NA>          NA     NA    NA  8.49e- 2       NA
      252       8          255 <NA>          NA     NA    NA  2.34e- 2       NA
      253       8          256 <NA>          NA     NA    NA  4.02e- 3       NA
      254       8          257 <NA>          NA     NA    NA  4.30e- 4       NA
      255       8          258 <NA>          NA     NA    NA  2.85e- 5       NA
      256       8          259 <NA>          NA     NA    NA  1.17e- 6       NA
      257       8          260 <NA>          NA     NA    NA  3.04e- 8       NA
      258       8          261 <NA>          NA     NA    NA  6.22e-21       NA
      259       8          262 <NA>          NA     NA    NA  3.17e-18       NA
      260       8          263 <NA>          NA     NA    NA  1.02e-15       NA
      261       8          264 <NA>          NA     NA    NA  2.04e-13       NA
      262       8          265 <NA>          NA     NA    NA  2.58e-11       NA
      263       8          266 <NA>          NA     NA    NA  2.05e- 9       NA
      264       8          267 <NA>          NA     NA    NA  1.03e- 7       NA
      265       8          268 <NA>          NA     NA    NA  3.23e- 6       NA
      266       8          269 <NA>          NA     NA    NA  6.43e- 5       NA
      267       8          270 <NA>          NA     NA    NA  8.07e- 4       NA
      268       8          271 <NA>          NA     NA    NA  6.41e- 3       NA
      269       8          272 <NA>          NA     NA    NA  3.23e- 2       NA
      270       8          273 <NA>          NA     NA    NA  1.03e- 1       NA
      271       8          274 <NA>          NA     NA    NA  2.09e- 1       NA
      272       8          275 <NA>          NA     NA    NA  2.69e- 1       NA
      273       8          276 <NA>          NA     NA    NA  2.20e- 1       NA
      274       8          277 <NA>          NA     NA    NA  1.14e- 1       NA
      275       8          278 <NA>          NA     NA    NA  3.76e- 2       NA
      276       8          279 <NA>          NA     NA    NA  7.85e- 3       NA
      277       8          280 <NA>          NA     NA    NA  1.04e- 3       NA
      278       8          281 <NA>          NA     NA    NA  8.70e- 5       NA
      279       8          282 <NA>          NA     NA    NA  4.61e- 6       NA
      280       8          283 <NA>          NA     NA    NA  1.57e- 7       NA
      281       4          284 <NA>          NA     NA    NA -3.60e+ 0       NA
      282       4          285 <NA>          NA     NA    NA -3.10e+ 0       NA
      283       4          286 <NA>          NA     NA    NA -2.80e+ 0       NA
      284       4          287 <NA>          NA     NA    NA -3.02e+ 0       NA
      285       4          288 <NA>          NA     NA    NA -2.44e+ 0       NA
      286       4          289 <NA>          NA     NA    NA -2.43e+ 0       NA
      287       4          290 <NA>          NA     NA    NA -1.68e+ 0       NA
      288       4          291 <NA>          NA     NA    NA -2.22e+ 0       NA
      289       4          292 <NA>          NA     NA    NA -2.02e+ 0       NA
      290       4          293 <NA>          NA     NA    NA -1.89e+ 0       NA
      291       4          294 <NA>          NA     NA    NA -1.82e+ 0       NA
      292       4          295 <NA>          NA     NA    NA -2.15e+ 0       NA
      293       4          296 <NA>          NA     NA    NA -1.78e+ 0       NA
      294       4          297 <NA>          NA     NA    NA -1.71e+ 0       NA
      295       4          298 <NA>          NA     NA    NA -1.82e+ 0       NA
      296       4          299 <NA>          NA     NA    NA -1.16e+ 0       NA
      297       4          300 <NA>          NA     NA    NA -1.36e+ 0       NA
      298       4          301 <NA>          NA     NA    NA -1.37e+ 0       NA
      299       4          302 <NA>          NA     NA    NA -1.38e+ 0       NA
      300       4          303 <NA>          NA     NA    NA -1.05e+ 0       NA
      301       4          304 <NA>          NA     NA    NA -1.37e+ 0       NA
      302       4          305 <NA>          NA     NA    NA -8.71e- 1       NA
      303       4          306 <NA>          NA     NA    NA -1.06e+ 0       NA
      304       4          307 <NA>          NA     NA    NA -1.07e+ 0       NA
      305       4          308 <NA>          NA     NA    NA -1.16e+ 0       NA
      306       4          309 <NA>          NA     NA    NA -1.18e+ 0       NA
      307       4          310 <NA>          NA     NA    NA -8.40e- 1       NA
      308       4          311 <NA>          NA     NA    NA -1.11e+ 0       NA
      309       4          312 <NA>          NA     NA    NA -6.94e- 1       NA
      310      16          349 <NA>          NA     NA    NA  1.5 e+ 0       NA
      311      17          350 <NA>          NA     NA    NA  2   e+ 0       NA
      312      18          351 <NA>          NA     NA    NA -1.49e+ 1       NA
      313      19          352 <NA>          NA     NA    NA  0              NA
      314      23          356 <NA>          NA     NA    NA  1.26e-16       NA
      315      23          357 <NA>          NA     NA    NA  8.39e-11       NA
      316      23          358 <NA>          NA     NA    NA  2.30e- 6       NA
      317      23          359 <NA>          NA     NA    NA  2.74e- 3       NA
      318      23          360 <NA>          NA     NA    NA  1.63e- 1       NA
      319      23          361 <NA>          NA     NA    NA  6.32e- 1       NA
      320      23          362 <NA>          NA     NA    NA  1.98e- 1       NA
          log_like_cv estimated
                <dbl> <lgl>    
        1          NA TRUE     
        2          NA TRUE     
        3          NA FALSE    
        4          NA TRUE     
        5          NA FALSE    
        6          NA FALSE    
        7          NA FALSE    
        8          NA FALSE    
        9          NA FALSE    
       10          NA FALSE    
       11          NA FALSE    
       12          NA FALSE    
       13          NA FALSE    
       14          NA FALSE    
       15          NA FALSE    
       16          NA FALSE    
       17          NA FALSE    
       18          NA FALSE    
       19          NA FALSE    
       20          NA FALSE    
       21          NA FALSE    
       22          NA FALSE    
       23          NA FALSE    
       24          NA FALSE    
       25          NA FALSE    
       26          NA FALSE    
       27          NA FALSE    
       28          NA FALSE    
       29          NA FALSE    
       30          NA FALSE    
       31          NA FALSE    
       32          NA FALSE    
       33          NA FALSE    
       34          NA FALSE    
       35          NA FALSE    
       36          NA FALSE    
       37          NA FALSE    
       38          NA FALSE    
       39          NA FALSE    
       40          NA FALSE    
       41          NA FALSE    
       42          NA FALSE    
       43          NA FALSE    
       44          NA FALSE    
       45          NA FALSE    
       46          NA FALSE    
       47          NA FALSE    
       48          NA FALSE    
       49          NA FALSE    
       50          NA FALSE    
       51          NA FALSE    
       52          NA FALSE    
       53          NA FALSE    
       54          NA FALSE    
       55          NA FALSE    
       56          NA FALSE    
       57          NA FALSE    
       58          NA FALSE    
       59          NA FALSE    
       60          NA FALSE    
       61          NA FALSE    
       62          NA FALSE    
       63          NA FALSE    
       64          NA FALSE    
       65          NA FALSE    
       66          NA FALSE    
       67          NA FALSE    
       68          NA FALSE    
       69          NA FALSE    
       70          NA FALSE    
       71          NA FALSE    
       72          NA FALSE    
       73          NA FALSE    
       74          NA FALSE    
       75          NA FALSE    
       76          NA FALSE    
       77          NA FALSE    
       78          NA FALSE    
       79          NA FALSE    
       80          NA FALSE    
       81          NA FALSE    
       82          NA FALSE    
       83          NA FALSE    
       84          NA FALSE    
       85          NA FALSE    
       86          NA FALSE    
       87          NA FALSE    
       88          NA FALSE    
       89          NA FALSE    
       90          NA FALSE    
       91          NA FALSE    
       92          NA FALSE    
       93          NA FALSE    
       94          NA FALSE    
       95          NA FALSE    
       96          NA FALSE    
       97          NA FALSE    
       98          NA FALSE    
       99          NA FALSE    
      100          NA FALSE    
      101          NA FALSE    
      102          NA FALSE    
      103          NA FALSE    
      104          NA FALSE    
      105          NA FALSE    
      106          NA FALSE    
      107          NA FALSE    
      108          NA FALSE    
      109          NA FALSE    
      110          NA FALSE    
      111          NA FALSE    
      112          NA FALSE    
      113          NA FALSE    
      114          NA FALSE    
      115          NA FALSE    
      116          NA FALSE    
      117          NA FALSE    
      118          NA FALSE    
      119          NA FALSE    
      120          NA FALSE    
      121          NA FALSE    
      122          NA FALSE    
      123          NA FALSE    
      124          NA FALSE    
      125          NA FALSE    
      126          NA FALSE    
      127          NA FALSE    
      128          NA FALSE    
      129          NA FALSE    
      130          NA FALSE    
      131          NA FALSE    
      132          NA FALSE    
      133          NA FALSE    
      134          NA FALSE    
      135          NA FALSE    
      136          NA FALSE    
      137          NA FALSE    
      138          NA FALSE    
      139          NA FALSE    
      140          NA FALSE    
      141          NA FALSE    
      142          NA FALSE    
      143          NA FALSE    
      144          NA FALSE    
      145          NA FALSE    
      146          NA FALSE    
      147          NA FALSE    
      148          NA FALSE    
      149          NA FALSE    
      150          NA FALSE    
      151          NA FALSE    
      152          NA FALSE    
      153          NA FALSE    
      154          NA FALSE    
      155          NA FALSE    
      156          NA FALSE    
      157          NA FALSE    
      158          NA FALSE    
      159          NA FALSE    
      160          NA FALSE    
      161          NA FALSE    
      162          NA FALSE    
      163          NA FALSE    
      164          NA FALSE    
      165          NA FALSE    
      166          NA FALSE    
      167          NA FALSE    
      168          NA FALSE    
      169          NA FALSE    
      170          NA FALSE    
      171          NA FALSE    
      172          NA FALSE    
      173          NA FALSE    
      174          NA FALSE    
      175          NA FALSE    
      176          NA FALSE    
      177          NA FALSE    
      178          NA FALSE    
      179          NA FALSE    
      180          NA FALSE    
      181          NA FALSE    
      182          NA FALSE    
      183          NA FALSE    
      184          NA FALSE    
      185          NA FALSE    
      186          NA FALSE    
      187          NA FALSE    
      188          NA FALSE    
      189          NA FALSE    
      190          NA FALSE    
      191          NA FALSE    
      192          NA FALSE    
      193          NA FALSE    
      194          NA FALSE    
      195          NA FALSE    
      196          NA FALSE    
      197          NA FALSE    
      198          NA FALSE    
      199          NA FALSE    
      200          NA FALSE    
      201          NA FALSE    
      202          NA FALSE    
      203          NA FALSE    
      204          NA FALSE    
      205          NA FALSE    
      206          NA FALSE    
      207          NA FALSE    
      208          NA FALSE    
      209          NA FALSE    
      210          NA FALSE    
      211          NA FALSE    
      212          NA FALSE    
      213          NA FALSE    
      214          NA FALSE    
      215          NA FALSE    
      216          NA FALSE    
      217          NA FALSE    
      218          NA FALSE    
      219          NA FALSE    
      220          NA FALSE    
      221          NA FALSE    
      222          NA FALSE    
      223          NA FALSE    
      224          NA FALSE    
      225          NA FALSE    
      226          NA FALSE    
      227          NA FALSE    
      228          NA FALSE    
      229          NA FALSE    
      230          NA FALSE    
      231          NA FALSE    
      232          NA FALSE    
      233          NA FALSE    
      234          NA FALSE    
      235          NA FALSE    
      236          NA FALSE    
      237          NA FALSE    
      238          NA FALSE    
      239          NA FALSE    
      240          NA FALSE    
      241          NA FALSE    
      242          NA FALSE    
      243          NA FALSE    
      244          NA FALSE    
      245          NA FALSE    
      246          NA FALSE    
      247          NA FALSE    
      248          NA FALSE    
      249          NA FALSE    
      250          NA FALSE    
      251          NA FALSE    
      252          NA FALSE    
      253          NA FALSE    
      254          NA FALSE    
      255          NA FALSE    
      256          NA FALSE    
      257          NA FALSE    
      258          NA FALSE    
      259          NA FALSE    
      260          NA FALSE    
      261          NA FALSE    
      262          NA FALSE    
      263          NA FALSE    
      264          NA FALSE    
      265          NA FALSE    
      266          NA FALSE    
      267          NA FALSE    
      268          NA FALSE    
      269          NA FALSE    
      270          NA FALSE    
      271          NA FALSE    
      272          NA FALSE    
      273          NA FALSE    
      274          NA FALSE    
      275          NA FALSE    
      276          NA FALSE    
      277          NA FALSE    
      278          NA FALSE    
      279          NA FALSE    
      280          NA FALSE    
      281          NA TRUE     
      282          NA TRUE     
      283          NA TRUE     
      284          NA TRUE     
      285          NA TRUE     
      286          NA TRUE     
      287          NA TRUE     
      288          NA TRUE     
      289          NA TRUE     
      290          NA TRUE     
      291          NA TRUE     
      292          NA TRUE     
      293          NA TRUE     
      294          NA TRUE     
      295          NA TRUE     
      296          NA TRUE     
      297          NA TRUE     
      298          NA TRUE     
      299          NA TRUE     
      300          NA TRUE     
      301          NA TRUE     
      302          NA TRUE     
      303          NA TRUE     
      304          NA TRUE     
      305          NA TRUE     
      306          NA TRUE     
      307          NA TRUE     
      308          NA TRUE     
      309          NA TRUE     
      310          NA TRUE     
      311          NA TRUE     
      312          NA TRUE     
      313          NA FALSE    
      314          NA FALSE    
      315          NA FALSE    
      316          NA FALSE    
      317          NA FALSE    
      318          NA FALSE    
      319          NA FALSE    
      320          NA FALSE    
      # i 6,210 more rows

