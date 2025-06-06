# get_estimates() works with deterministic run

    Code
      print(dplyr::select(get_estimates(deterministic_results), -estimate,
      -uncertainty, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 11,765 x 15
      # Groups:   label [107]
          module_name module_id module_type label                    type   type_id
          <chr>           <int> <chr>       <chr>                    <chr>    <int>
        1 selectivity         1 Logistic    inflection_point         vector       1
        2 selectivity         1 Logistic    slope                    vector       2
        3 Fleet               1 fleet       log_q                    vector       3
        4 Fleet               1 fleet       log_Fmort                vector       4
        5 Fleet               1 fleet       age_to_length_conversion vector      11
        6 Fleet               1 fleet       age_to_length_conversion vector      11
        7 Fleet               1 fleet       age_to_length_conversion vector      11
        8 Fleet               1 fleet       age_to_length_conversion vector      11
        9 Fleet               1 fleet       age_to_length_conversion vector      11
       10 Fleet               1 fleet       age_to_length_conversion vector      11
       11 Fleet               1 fleet       age_to_length_conversion vector      11
       12 Fleet               1 fleet       age_to_length_conversion vector      11
       13 Fleet               1 fleet       age_to_length_conversion vector      11
       14 Fleet               1 fleet       age_to_length_conversion vector      11
       15 Fleet               1 fleet       age_to_length_conversion vector      11
       16 Fleet               1 fleet       age_to_length_conversion vector      11
       17 Fleet               1 fleet       age_to_length_conversion vector      11
       18 Fleet               1 fleet       age_to_length_conversion vector      11
       19 Fleet               1 fleet       age_to_length_conversion vector      11
       20 Fleet               1 fleet       age_to_length_conversion vector      11
       21 Fleet               1 fleet       age_to_length_conversion vector      11
       22 Fleet               1 fleet       age_to_length_conversion vector      11
       23 Fleet               1 fleet       age_to_length_conversion vector      11
       24 Fleet               1 fleet       age_to_length_conversion vector      11
       25 Fleet               1 fleet       age_to_length_conversion vector      11
       26 Fleet               1 fleet       age_to_length_conversion vector      11
       27 Fleet               1 fleet       age_to_length_conversion vector      11
       28 Fleet               1 fleet       age_to_length_conversion vector      11
       29 Fleet               1 fleet       age_to_length_conversion vector      11
       30 Fleet               1 fleet       age_to_length_conversion vector      11
       31 Fleet               1 fleet       age_to_length_conversion vector      11
       32 Fleet               1 fleet       age_to_length_conversion vector      11
       33 Fleet               1 fleet       age_to_length_conversion vector      11
       34 Fleet               1 fleet       age_to_length_conversion vector      11
       35 Fleet               1 fleet       age_to_length_conversion vector      11
       36 Fleet               1 fleet       age_to_length_conversion vector      11
       37 Fleet               1 fleet       age_to_length_conversion vector      11
       38 Fleet               1 fleet       age_to_length_conversion vector      11
       39 Fleet               1 fleet       age_to_length_conversion vector      11
       40 Fleet               1 fleet       age_to_length_conversion vector      11
       41 Fleet               1 fleet       age_to_length_conversion vector      11
       42 Fleet               1 fleet       age_to_length_conversion vector      11
       43 Fleet               1 fleet       age_to_length_conversion vector      11
       44 Fleet               1 fleet       age_to_length_conversion vector      11
       45 Fleet               1 fleet       age_to_length_conversion vector      11
       46 Fleet               1 fleet       age_to_length_conversion vector      11
       47 Fleet               1 fleet       age_to_length_conversion vector      11
       48 Fleet               1 fleet       age_to_length_conversion vector      11
       49 Fleet               1 fleet       age_to_length_conversion vector      11
       50 Fleet               1 fleet       age_to_length_conversion vector      11
       51 Fleet               1 fleet       age_to_length_conversion vector      11
       52 Fleet               1 fleet       age_to_length_conversion vector      11
       53 Fleet               1 fleet       age_to_length_conversion vector      11
       54 Fleet               1 fleet       age_to_length_conversion vector      11
       55 Fleet               1 fleet       age_to_length_conversion vector      11
       56 Fleet               1 fleet       age_to_length_conversion vector      11
       57 Fleet               1 fleet       age_to_length_conversion vector      11
       58 Fleet               1 fleet       age_to_length_conversion vector      11
       59 Fleet               1 fleet       age_to_length_conversion vector      11
       60 Fleet               1 fleet       age_to_length_conversion vector      11
       61 Fleet               1 fleet       age_to_length_conversion vector      11
       62 Fleet               1 fleet       age_to_length_conversion vector      11
       63 Fleet               1 fleet       age_to_length_conversion vector      11
       64 Fleet               1 fleet       age_to_length_conversion vector      11
       65 Fleet               1 fleet       age_to_length_conversion vector      11
       66 Fleet               1 fleet       age_to_length_conversion vector      11
       67 Fleet               1 fleet       age_to_length_conversion vector      11
       68 Fleet               1 fleet       age_to_length_conversion vector      11
       69 Fleet               1 fleet       age_to_length_conversion vector      11
       70 Fleet               1 fleet       age_to_length_conversion vector      11
       71 Fleet               1 fleet       age_to_length_conversion vector      11
       72 Fleet               1 fleet       age_to_length_conversion vector      11
       73 Fleet               1 fleet       age_to_length_conversion vector      11
       74 Fleet               1 fleet       age_to_length_conversion vector      11
       75 Fleet               1 fleet       age_to_length_conversion vector      11
       76 Fleet               1 fleet       age_to_length_conversion vector      11
       77 Fleet               1 fleet       age_to_length_conversion vector      11
       78 Fleet               1 fleet       age_to_length_conversion vector      11
       79 Fleet               1 fleet       age_to_length_conversion vector      11
       80 Fleet               1 fleet       age_to_length_conversion vector      11
       81 Fleet               1 fleet       age_to_length_conversion vector      11
       82 Fleet               1 fleet       age_to_length_conversion vector      11
       83 Fleet               1 fleet       age_to_length_conversion vector      11
       84 Fleet               1 fleet       age_to_length_conversion vector      11
       85 Fleet               1 fleet       age_to_length_conversion vector      11
       86 Fleet               1 fleet       age_to_length_conversion vector      11
       87 Fleet               1 fleet       age_to_length_conversion vector      11
       88 Fleet               1 fleet       age_to_length_conversion vector      11
       89 Fleet               1 fleet       age_to_length_conversion vector      11
       90 Fleet               1 fleet       age_to_length_conversion vector      11
       91 Fleet               1 fleet       age_to_length_conversion vector      11
       92 Fleet               1 fleet       age_to_length_conversion vector      11
       93 Fleet               1 fleet       age_to_length_conversion vector      11
       94 Fleet               1 fleet       age_to_length_conversion vector      11
       95 Fleet               1 fleet       age_to_length_conversion vector      11
       96 Fleet               1 fleet       age_to_length_conversion vector      11
       97 Fleet               1 fleet       age_to_length_conversion vector      11
       98 Fleet               1 fleet       age_to_length_conversion vector      11
       99 Fleet               1 fleet       age_to_length_conversion vector      11
      100 Fleet               1 fleet       age_to_length_conversion vector      11
      101 Fleet               1 fleet       age_to_length_conversion vector      11
      102 Fleet               1 fleet       age_to_length_conversion vector      11
      103 Fleet               1 fleet       age_to_length_conversion vector      11
      104 Fleet               1 fleet       age_to_length_conversion vector      11
      105 Fleet               1 fleet       age_to_length_conversion vector      11
      106 Fleet               1 fleet       age_to_length_conversion vector      11
      107 Fleet               1 fleet       age_to_length_conversion vector      11
      108 Fleet               1 fleet       age_to_length_conversion vector      11
      109 Fleet               1 fleet       age_to_length_conversion vector      11
      110 Fleet               1 fleet       age_to_length_conversion vector      11
      111 Fleet               1 fleet       age_to_length_conversion vector      11
      112 Fleet               1 fleet       age_to_length_conversion vector      11
      113 Fleet               1 fleet       age_to_length_conversion vector      11
      114 Fleet               1 fleet       age_to_length_conversion vector      11
      115 Fleet               1 fleet       age_to_length_conversion vector      11
      116 Fleet               1 fleet       age_to_length_conversion vector      11
      117 Fleet               1 fleet       age_to_length_conversion vector      11
      118 Fleet               1 fleet       age_to_length_conversion vector      11
      119 Fleet               1 fleet       age_to_length_conversion vector      11
      120 Fleet               1 fleet       age_to_length_conversion vector      11
      121 Fleet               1 fleet       age_to_length_conversion vector      11
      122 Fleet               1 fleet       age_to_length_conversion vector      11
      123 Fleet               1 fleet       age_to_length_conversion vector      11
      124 Fleet               1 fleet       age_to_length_conversion vector      11
      125 Fleet               1 fleet       age_to_length_conversion vector      11
      126 Fleet               1 fleet       age_to_length_conversion vector      11
      127 Fleet               1 fleet       age_to_length_conversion vector      11
      128 Fleet               1 fleet       age_to_length_conversion vector      11
      129 Fleet               1 fleet       age_to_length_conversion vector      11
      130 Fleet               1 fleet       age_to_length_conversion vector      11
      131 Fleet               1 fleet       age_to_length_conversion vector      11
      132 Fleet               1 fleet       age_to_length_conversion vector      11
      133 Fleet               1 fleet       age_to_length_conversion vector      11
      134 Fleet               1 fleet       age_to_length_conversion vector      11
      135 Fleet               1 fleet       age_to_length_conversion vector      11
      136 Fleet               1 fleet       age_to_length_conversion vector      11
      137 Fleet               1 fleet       age_to_length_conversion vector      11
      138 Fleet               1 fleet       age_to_length_conversion vector      11
      139 Fleet               1 fleet       age_to_length_conversion vector      11
      140 Fleet               1 fleet       age_to_length_conversion vector      11
      141 Fleet               1 fleet       age_to_length_conversion vector      11
      142 Fleet               1 fleet       age_to_length_conversion vector      11
      143 Fleet               1 fleet       age_to_length_conversion vector      11
      144 Fleet               1 fleet       age_to_length_conversion vector      11
      145 Fleet               1 fleet       age_to_length_conversion vector      11
      146 Fleet               1 fleet       age_to_length_conversion vector      11
      147 Fleet               1 fleet       age_to_length_conversion vector      11
      148 Fleet               1 fleet       age_to_length_conversion vector      11
      149 Fleet               1 fleet       age_to_length_conversion vector      11
      150 Fleet               1 fleet       age_to_length_conversion vector      11
      151 Fleet               1 fleet       age_to_length_conversion vector      11
      152 Fleet               1 fleet       age_to_length_conversion vector      11
      153 Fleet               1 fleet       age_to_length_conversion vector      11
      154 Fleet               1 fleet       age_to_length_conversion vector      11
      155 Fleet               1 fleet       age_to_length_conversion vector      11
      156 Fleet               1 fleet       age_to_length_conversion vector      11
      157 Fleet               1 fleet       age_to_length_conversion vector      11
      158 Fleet               1 fleet       age_to_length_conversion vector      11
      159 Fleet               1 fleet       age_to_length_conversion vector      11
      160 Fleet               1 fleet       age_to_length_conversion vector      11
      161 Fleet               1 fleet       age_to_length_conversion vector      11
      162 Fleet               1 fleet       age_to_length_conversion vector      11
      163 Fleet               1 fleet       age_to_length_conversion vector      11
      164 Fleet               1 fleet       age_to_length_conversion vector      11
      165 Fleet               1 fleet       age_to_length_conversion vector      11
      166 Fleet               1 fleet       age_to_length_conversion vector      11
      167 Fleet               1 fleet       age_to_length_conversion vector      11
      168 Fleet               1 fleet       age_to_length_conversion vector      11
      169 Fleet               1 fleet       age_to_length_conversion vector      11
      170 Fleet               1 fleet       age_to_length_conversion vector      11
      171 Fleet               1 fleet       age_to_length_conversion vector      11
      172 Fleet               1 fleet       age_to_length_conversion vector      11
      173 Fleet               1 fleet       age_to_length_conversion vector      11
      174 Fleet               1 fleet       age_to_length_conversion vector      11
      175 Fleet               1 fleet       age_to_length_conversion vector      11
      176 Fleet               1 fleet       age_to_length_conversion vector      11
      177 Fleet               1 fleet       age_to_length_conversion vector      11
      178 Fleet               1 fleet       age_to_length_conversion vector      11
      179 Fleet               1 fleet       age_to_length_conversion vector      11
      180 Fleet               1 fleet       age_to_length_conversion vector      11
      181 Fleet               1 fleet       age_to_length_conversion vector      11
      182 Fleet               1 fleet       age_to_length_conversion vector      11
      183 Fleet               1 fleet       age_to_length_conversion vector      11
      184 Fleet               1 fleet       age_to_length_conversion vector      11
      185 Fleet               1 fleet       age_to_length_conversion vector      11
      186 Fleet               1 fleet       age_to_length_conversion vector      11
      187 Fleet               1 fleet       age_to_length_conversion vector      11
      188 Fleet               1 fleet       age_to_length_conversion vector      11
      189 Fleet               1 fleet       age_to_length_conversion vector      11
      190 Fleet               1 fleet       age_to_length_conversion vector      11
      191 Fleet               1 fleet       age_to_length_conversion vector      11
      192 Fleet               1 fleet       age_to_length_conversion vector      11
      193 Fleet               1 fleet       age_to_length_conversion vector      11
      194 Fleet               1 fleet       age_to_length_conversion vector      11
      195 Fleet               1 fleet       age_to_length_conversion vector      11
      196 Fleet               1 fleet       age_to_length_conversion vector      11
      197 Fleet               1 fleet       age_to_length_conversion vector      11
      198 Fleet               1 fleet       age_to_length_conversion vector      11
      199 Fleet               1 fleet       age_to_length_conversion vector      11
      200 Fleet               1 fleet       age_to_length_conversion vector      11
      201 Fleet               1 fleet       age_to_length_conversion vector      11
      202 Fleet               1 fleet       age_to_length_conversion vector      11
      203 Fleet               1 fleet       age_to_length_conversion vector      11
      204 Fleet               1 fleet       age_to_length_conversion vector      11
      205 Fleet               1 fleet       age_to_length_conversion vector      11
      206 Fleet               1 fleet       age_to_length_conversion vector      11
      207 Fleet               1 fleet       age_to_length_conversion vector      11
      208 Fleet               1 fleet       age_to_length_conversion vector      11
      209 Fleet               1 fleet       age_to_length_conversion vector      11
      210 Fleet               1 fleet       age_to_length_conversion vector      11
      211 Fleet               1 fleet       age_to_length_conversion vector      11
      212 Fleet               1 fleet       age_to_length_conversion vector      11
      213 Fleet               1 fleet       age_to_length_conversion vector      11
      214 Fleet               1 fleet       age_to_length_conversion vector      11
      215 Fleet               1 fleet       age_to_length_conversion vector      11
      216 Fleet               1 fleet       age_to_length_conversion vector      11
      217 Fleet               1 fleet       age_to_length_conversion vector      11
      218 Fleet               1 fleet       age_to_length_conversion vector      11
      219 Fleet               1 fleet       age_to_length_conversion vector      11
      220 Fleet               1 fleet       age_to_length_conversion vector      11
      221 Fleet               1 fleet       age_to_length_conversion vector      11
      222 Fleet               1 fleet       age_to_length_conversion vector      11
      223 Fleet               1 fleet       age_to_length_conversion vector      11
      224 Fleet               1 fleet       age_to_length_conversion vector      11
      225 Fleet               1 fleet       age_to_length_conversion vector      11
      226 Fleet               1 fleet       age_to_length_conversion vector      11
      227 Fleet               1 fleet       age_to_length_conversion vector      11
      228 Fleet               1 fleet       age_to_length_conversion vector      11
      229 Fleet               1 fleet       age_to_length_conversion vector      11
      230 Fleet               1 fleet       age_to_length_conversion vector      11
      231 Fleet               1 fleet       age_to_length_conversion vector      11
      232 Fleet               1 fleet       age_to_length_conversion vector      11
      233 Fleet               1 fleet       age_to_length_conversion vector      11
      234 Fleet               1 fleet       age_to_length_conversion vector      11
      235 Fleet               1 fleet       age_to_length_conversion vector      11
      236 Fleet               1 fleet       age_to_length_conversion vector      11
      237 Fleet               1 fleet       age_to_length_conversion vector      11
      238 Fleet               1 fleet       age_to_length_conversion vector      11
      239 Fleet               1 fleet       age_to_length_conversion vector      11
      240 Fleet               1 fleet       age_to_length_conversion vector      11
      241 Fleet               1 fleet       age_to_length_conversion vector      11
      242 Fleet               1 fleet       age_to_length_conversion vector      11
      243 Fleet               1 fleet       age_to_length_conversion vector      11
      244 Fleet               1 fleet       age_to_length_conversion vector      11
      245 Fleet               1 fleet       age_to_length_conversion vector      11
      246 Fleet               1 fleet       age_to_length_conversion vector      11
      247 Fleet               1 fleet       age_to_length_conversion vector      11
      248 Fleet               1 fleet       age_to_length_conversion vector      11
      249 Fleet               1 fleet       age_to_length_conversion vector      11
      250 Fleet               1 fleet       age_to_length_conversion vector      11
      251 Fleet               1 fleet       age_to_length_conversion vector      11
      252 Fleet               1 fleet       age_to_length_conversion vector      11
      253 Fleet               1 fleet       age_to_length_conversion vector      11
      254 Fleet               1 fleet       age_to_length_conversion vector      11
      255 Fleet               1 fleet       age_to_length_conversion vector      11
      256 Fleet               1 fleet       age_to_length_conversion vector      11
      257 Fleet               1 fleet       age_to_length_conversion vector      11
      258 Fleet               1 fleet       age_to_length_conversion vector      11
      259 Fleet               1 fleet       age_to_length_conversion vector      11
      260 Fleet               1 fleet       age_to_length_conversion vector      11
      261 Fleet               1 fleet       age_to_length_conversion vector      11
      262 Fleet               1 fleet       age_to_length_conversion vector      11
      263 Fleet               1 fleet       age_to_length_conversion vector      11
      264 Fleet               1 fleet       age_to_length_conversion vector      11
      265 Fleet               1 fleet       age_to_length_conversion vector      11
      266 Fleet               1 fleet       age_to_length_conversion vector      11
      267 Fleet               1 fleet       age_to_length_conversion vector      11
      268 Fleet               1 fleet       age_to_length_conversion vector      11
      269 Fleet               1 fleet       age_to_length_conversion vector      11
      270 Fleet               1 fleet       age_to_length_conversion vector      11
      271 Fleet               1 fleet       age_to_length_conversion vector      11
      272 Fleet               1 fleet       age_to_length_conversion vector      11
      273 Fleet               1 fleet       age_to_length_conversion vector      11
      274 Fleet               1 fleet       age_to_length_conversion vector      11
      275 Fleet               1 fleet       age_to_length_conversion vector      11
      276 Fleet               1 fleet       age_to_length_conversion vector      11
      277 Fleet               1 fleet       age_to_length_conversion vector      11
      278 Fleet               1 fleet       age_to_length_conversion vector      11
      279 Fleet               1 fleet       age_to_length_conversion vector      11
      280 Fleet               1 fleet       age_to_length_conversion vector      11
      281 Fleet               1 fleet       log_Fmort                vector       4
      282 Fleet               1 fleet       log_Fmort                vector       4
      283 Fleet               1 fleet       log_Fmort                vector       4
      284 Fleet               1 fleet       log_Fmort                vector       4
      285 Fleet               1 fleet       log_Fmort                vector       4
      286 Fleet               1 fleet       log_Fmort                vector       4
      287 Fleet               1 fleet       log_Fmort                vector       4
      288 Fleet               1 fleet       log_Fmort                vector       4
      289 Fleet               1 fleet       log_Fmort                vector       4
      290 Fleet               1 fleet       log_Fmort                vector       4
      291 Fleet               1 fleet       log_Fmort                vector       4
      292 Fleet               1 fleet       log_Fmort                vector       4
      293 Fleet               1 fleet       log_Fmort                vector       4
      294 Fleet               1 fleet       log_Fmort                vector       4
      295 Fleet               1 fleet       log_Fmort                vector       4
      296 Fleet               1 fleet       log_Fmort                vector       4
      297 Fleet               1 fleet       log_Fmort                vector       4
      298 Fleet               1 fleet       log_Fmort                vector       4
      299 Fleet               1 fleet       log_Fmort                vector       4
      300 Fleet               1 fleet       log_Fmort                vector       4
      301 Fleet               1 fleet       log_Fmort                vector       4
      302 Fleet               1 fleet       log_Fmort                vector       4
      303 Fleet               1 fleet       log_Fmort                vector       4
      304 Fleet               1 fleet       log_Fmort                vector       4
      305 Fleet               1 fleet       log_Fmort                vector       4
      306 Fleet               1 fleet       log_Fmort                vector       4
      307 Fleet               1 fleet       log_Fmort                vector       4
      308 Fleet               1 fleet       log_Fmort                vector       4
      309 Fleet               1 fleet       log_Fmort                vector       4
      310 selectivity         2 Logistic    inflection_point         vector      19
      311 selectivity         2 Logistic    slope                    vector      20
      312 Fleet               2 fleet       log_q                    vector      21
      313 Fleet               2 fleet       log_Fmort                vector      22
      314 Fleet               2 fleet       age_to_length_conversion vector      29
      315 Fleet               2 fleet       age_to_length_conversion vector      29
      316 Fleet               2 fleet       age_to_length_conversion vector      29
      317 Fleet               2 fleet       age_to_length_conversion vector      29
      318 Fleet               2 fleet       age_to_length_conversion vector      29
      319 Fleet               2 fleet       age_to_length_conversion vector      29
      320 Fleet               2 fleet       age_to_length_conversion vector      29
          parameter_id fleet_name   age length  time   initial log_like log_like_cv
                 <int> <chr>      <dbl>  <dbl> <int>     <dbl>    <dbl>       <dbl>
        1            1 <NA>          NA     NA    NA  2   e+ 0       NA          NA
        2            2 <NA>          NA     NA    NA  1   e+ 0       NA          NA
        3            3 <NA>          NA     NA    NA  0              NA          NA
        4            4 <NA>          NA     NA    NA -4.66e+ 0       NA          NA
        5           11 <NA>          NA     NA    NA  1.26e-16       NA          NA
        6           12 <NA>          NA     NA    NA  8.39e-11       NA          NA
        7           13 <NA>          NA     NA    NA  2.30e- 6       NA          NA
        8           14 <NA>          NA     NA    NA  2.74e- 3       NA          NA
        9           15 <NA>          NA     NA    NA  1.63e- 1       NA          NA
       10           16 <NA>          NA     NA    NA  6.32e- 1       NA          NA
       11           17 <NA>          NA     NA    NA  1.98e- 1       NA          NA
       12           18 <NA>          NA     NA    NA  4.13e- 3       NA          NA
       13           19 <NA>          NA     NA    NA  4.36e- 6       NA          NA
       14           20 <NA>          NA     NA    NA  2.02e-10       NA          NA
       15           21 <NA>          NA     NA    NA  3.33e-16       NA          NA
       16           22 <NA>          NA     NA    NA  0              NA          NA
       17           23 <NA>          NA     NA    NA  0              NA          NA
       18           24 <NA>          NA     NA    NA  0              NA          NA
       19           25 <NA>          NA     NA    NA  0              NA          NA
       20           26 <NA>          NA     NA    NA  0              NA          NA
       21           27 <NA>          NA     NA    NA  0              NA          NA
       22           28 <NA>          NA     NA    NA  0              NA          NA
       23           29 <NA>          NA     NA    NA  0              NA          NA
       24           30 <NA>          NA     NA    NA  0              NA          NA
       25           31 <NA>          NA     NA    NA  0              NA          NA
       26           32 <NA>          NA     NA    NA  0              NA          NA
       27           33 <NA>          NA     NA    NA  0              NA          NA
       28           34 <NA>          NA     NA    NA  3.27e-18       NA          NA
       29           35 <NA>          NA     NA    NA  2.15e-13       NA          NA
       30           36 <NA>          NA     NA    NA  2.20e- 9       NA          NA
       31           37 <NA>          NA     NA    NA  3.54e- 6       NA          NA
       32           38 <NA>          NA     NA    NA  9.19e- 4       NA          NA
       33           39 <NA>          NA     NA    NA  4.03e- 2       NA          NA
       34           40 <NA>          NA     NA    NA  3.18e- 1       NA          NA
       35           41 <NA>          NA     NA    NA  4.86e- 1       NA          NA
       36           42 <NA>          NA     NA    NA  1.46e- 1       NA          NA
       37           43 <NA>          NA     NA    NA  8.23e- 3       NA          NA
       38           44 <NA>          NA     NA    NA  8.08e- 5       NA          NA
       39           45 <NA>          NA     NA    NA  1.31e- 7       NA          NA
       40           46 <NA>          NA     NA    NA  3.37e-11       NA          NA
       41           47 <NA>          NA     NA    NA  1.33e-15       NA          NA
       42           48 <NA>          NA     NA    NA  0              NA          NA
       43           49 <NA>          NA     NA    NA  0              NA          NA
       44           50 <NA>          NA     NA    NA  0              NA          NA
       45           51 <NA>          NA     NA    NA  0              NA          NA
       46           52 <NA>          NA     NA    NA  0              NA          NA
       47           53 <NA>          NA     NA    NA  0              NA          NA
       48           54 <NA>          NA     NA    NA  0              NA          NA
       49           55 <NA>          NA     NA    NA  0              NA          NA
       50           56 <NA>          NA     NA    NA  0              NA          NA
       51           57 <NA>          NA     NA    NA  4.35e-19       NA          NA
       52           58 <NA>          NA     NA    NA  6.73e-15       NA          NA
       53           59 <NA>          NA     NA    NA  2.84e-11       NA          NA
       54           60 <NA>          NA     NA    NA  3.28e- 8       NA          NA
       55           61 <NA>          NA     NA    NA  1.05e- 5       NA          NA
       56           62 <NA>          NA     NA    NA  9.44e- 4       NA          NA
       57           63 <NA>          NA     NA    NA  2.44e- 2       NA          NA
       58           64 <NA>          NA     NA    NA  1.85e- 1       NA          NA
       59           65 <NA>          NA     NA    NA  4.24e- 1       NA          NA
       60           66 <NA>          NA     NA    NA  2.98e- 1       NA          NA
       61           67 <NA>          NA     NA    NA  6.36e- 2       NA          NA
       62           68 <NA>          NA     NA    NA  4.04e- 3       NA          NA
       63           69 <NA>          NA     NA    NA  7.43e- 5       NA          NA
       64           70 <NA>          NA     NA    NA  3.87e- 7       NA          NA
       65           71 <NA>          NA     NA    NA  5.62e-10       NA          NA
       66           72 <NA>          NA     NA    NA  2.24e-13       NA          NA
       67           73 <NA>          NA     NA    NA  0              NA          NA
       68           74 <NA>          NA     NA    NA  0              NA          NA
       69           75 <NA>          NA     NA    NA  0              NA          NA
       70           76 <NA>          NA     NA    NA  0              NA          NA
       71           77 <NA>          NA     NA    NA  0              NA          NA
       72           78 <NA>          NA     NA    NA  0              NA          NA
       73           79 <NA>          NA     NA    NA  0              NA          NA
       74           80 <NA>          NA     NA    NA  1.23e-19       NA          NA
       75           81 <NA>          NA     NA    NA  7.29e-16       NA          NA
       76           82 <NA>          NA     NA    NA  1.58e-12       NA          NA
       77           83 <NA>          NA     NA    NA  1.25e- 9       NA          NA
       78           84 <NA>          NA     NA    NA  3.68e- 7       NA          NA
       79           85 <NA>          NA     NA    NA  4.02e- 5       NA          NA
       80           86 <NA>          NA     NA    NA  1.65e- 3       NA          NA
       81           87 <NA>          NA     NA    NA  2.56e- 2       NA          NA
       82           88 <NA>          NA     NA    NA  1.54e- 1       NA          NA
       83           89 <NA>          NA     NA    NA  3.58e- 1       NA          NA
       84           90 <NA>          NA     NA    NA  3.27e- 1       NA          NA
       85           91 <NA>          NA     NA    NA  1.17e- 1       NA          NA
       86           92 <NA>          NA     NA    NA  1.62e- 2       NA          NA
       87           93 <NA>          NA     NA    NA  8.65e- 4       NA          NA
       88           94 <NA>          NA     NA    NA  1.75e- 5       NA          NA
       89           95 <NA>          NA     NA    NA  1.32e- 7       NA          NA
       90           96 <NA>          NA     NA    NA  3.71e-10       NA          NA
       91           97 <NA>          NA     NA    NA  3.85e-13       NA          NA
       92           98 <NA>          NA     NA    NA  1.11e-16       NA          NA
       93           99 <NA>          NA     NA    NA  0              NA          NA
       94          100 <NA>          NA     NA    NA  0              NA          NA
       95          101 <NA>          NA     NA    NA  0              NA          NA
       96          102 <NA>          NA     NA    NA  0              NA          NA
       97          103 <NA>          NA     NA    NA  5.27e-20       NA          NA
       98          104 <NA>          NA     NA    NA  1.59e-16       NA          NA
       99          105 <NA>          NA     NA    NA  2.09e-13       NA          NA
      100          106 <NA>          NA     NA    NA  1.20e-10       NA          NA
      101          107 <NA>          NA     NA    NA  3.04e- 8       NA          NA
      102          108 <NA>          NA     NA    NA  3.38e- 6       NA          NA
      103          109 <NA>          NA     NA    NA  1.67e- 4       NA          NA
      104          110 <NA>          NA     NA    NA  3.67e- 3       NA          NA
      105          111 <NA>          NA     NA    NA  3.63e- 2       NA          NA
      106          112 <NA>          NA     NA    NA  1.63e- 1       NA          NA
      107          113 <NA>          NA     NA    NA  3.31e- 1       NA          NA
      108          114 <NA>          NA     NA    NA  3.08e- 1       NA          NA
      109          115 <NA>          NA     NA    NA  1.31e- 1       NA          NA
      110          116 <NA>          NA     NA    NA  2.52e- 2       NA          NA
      111          117 <NA>          NA     NA    NA  2.20e- 3       NA          NA
      112          118 <NA>          NA     NA    NA  8.62e- 5       NA          NA
      113          119 <NA>          NA     NA    NA  1.50e- 6       NA          NA
      114          120 <NA>          NA     NA    NA  1.16e- 8       NA          NA
      115          121 <NA>          NA     NA    NA  3.94e-11       NA          NA
      116          122 <NA>          NA     NA    NA  5.88e-14       NA          NA
      117          123 <NA>          NA     NA    NA  0              NA          NA
      118          124 <NA>          NA     NA    NA  0              NA          NA
      119          125 <NA>          NA     NA    NA  0              NA          NA
      120          126 <NA>          NA     NA    NA  2.88e-20       NA          NA
      121          127 <NA>          NA     NA    NA  5.32e-17       NA          NA
      122          128 <NA>          NA     NA    NA  4.81e-14       NA          NA
      123          129 <NA>          NA     NA    NA  2.13e-11       NA          NA
      124          130 <NA>          NA     NA    NA  4.62e- 9       NA          NA
      125          131 <NA>          NA     NA    NA  4.94e- 7       NA          NA
      126          132 <NA>          NA     NA    NA  2.61e- 5       NA          NA
      127          133 <NA>          NA     NA    NA  6.86e- 4       NA          NA
      128          134 <NA>          NA     NA    NA  8.98e- 3       NA          NA
      129          135 <NA>          NA     NA    NA  5.89e- 2       NA          NA
      130          136 <NA>          NA     NA    NA  1.94e- 1       NA          NA
      131          137 <NA>          NA     NA    NA  3.23e- 1       NA          NA
      132          138 <NA>          NA     NA    NA  2.72e- 1       NA          NA
      133          139 <NA>          NA     NA    NA  1.15e- 1       NA          NA
      134          140 <NA>          NA     NA    NA  2.47e- 2       NA          NA
      135          141 <NA>          NA     NA    NA  2.66e- 3       NA          NA
      136          142 <NA>          NA     NA    NA  1.43e- 4       NA          NA
      137          143 <NA>          NA     NA    NA  3.81e- 6       NA          NA
      138          144 <NA>          NA     NA    NA  5.04e- 8       NA          NA
      139          145 <NA>          NA     NA    NA  3.29e-10       NA          NA
      140          146 <NA>          NA     NA    NA  1.06e-12       NA          NA
      141          147 <NA>          NA     NA    NA  1.67e-15       NA          NA
      142          148 <NA>          NA     NA    NA  0              NA          NA
      143          149 <NA>          NA     NA    NA  1.85e-20       NA          NA
      144          150 <NA>          NA     NA    NA  2.36e-17       NA          NA
      145          151 <NA>          NA     NA    NA  1.60e-14       NA          NA
      146          152 <NA>          NA     NA    NA  5.73e-12       NA          NA
      147          153 <NA>          NA     NA    NA  1.09e- 9       NA          NA
      148          154 <NA>          NA     NA    NA  1.10e- 7       NA          NA
      149          155 <NA>          NA     NA    NA  5.93e- 6       NA          NA
      150          156 <NA>          NA     NA    NA  1.71e- 4       NA          NA
      151          157 <NA>          NA     NA    NA  2.63e- 3       NA          NA
      152          158 <NA>          NA     NA    NA  2.18e- 2       NA          NA
      153          159 <NA>          NA     NA    NA  9.79e- 2       NA          NA
      154          160 <NA>          NA     NA    NA  2.37e- 1       NA          NA
      155          161 <NA>          NA     NA    NA  3.12e- 1       NA          NA
      156          162 <NA>          NA     NA    NA  2.22e- 1       NA          NA
      157          163 <NA>          NA     NA    NA  8.59e- 2       NA          NA
      158          164 <NA>          NA     NA    NA  1.80e- 2       NA          NA
      159          165 <NA>          NA     NA    NA  2.03e- 3       NA          NA
      160          166 <NA>          NA     NA    NA  1.23e- 4       NA          NA
      161          167 <NA>          NA     NA    NA  4.00e- 6       NA          NA
      162          168 <NA>          NA     NA    NA  6.96e- 8       NA          NA
      163          169 <NA>          NA     NA    NA  6.44e-10       NA          NA
      164          170 <NA>          NA     NA    NA  3.17e-12       NA          NA
      165          171 <NA>          NA     NA    NA  8.33e-15       NA          NA
      166          172 <NA>          NA     NA    NA  1.32e-20       NA          NA
      167          173 <NA>          NA     NA    NA  1.28e-17       NA          NA
      168          174 <NA>          NA     NA    NA  6.90e-15       NA          NA
      169          175 <NA>          NA     NA    NA  2.09e-12       NA          NA
      170          176 <NA>          NA     NA    NA  3.54e-10       NA          NA
      171          177 <NA>          NA     NA    NA  3.37e- 8       NA          NA
      172          178 <NA>          NA     NA    NA  1.81e- 6       NA          NA
      173          179 <NA>          NA     NA    NA  5.45e- 5       NA          NA
      174          180 <NA>          NA     NA    NA  9.31e- 4       NA          NA
      175          181 <NA>          NA     NA    NA  9.00e- 3       NA          NA
      176          182 <NA>          NA     NA    NA  4.95e- 2       NA          NA
      177          183 <NA>          NA     NA    NA  1.55e- 1       NA          NA
      178          184 <NA>          NA     NA    NA  2.76e- 1       NA          NA
      179          185 <NA>          NA     NA    NA  2.81e- 1       NA          NA
      180          186 <NA>          NA     NA    NA  1.63e- 1       NA          NA
      181          187 <NA>          NA     NA    NA  5.42e- 2       NA          NA
      182          188 <NA>          NA     NA    NA  1.02e- 2       NA          NA
      183          189 <NA>          NA     NA    NA  1.10e- 3       NA          NA
      184          190 <NA>          NA     NA    NA  6.66e- 5       NA          NA
      185          191 <NA>          NA     NA    NA  2.29e- 6       NA          NA
      186          192 <NA>          NA     NA    NA  4.44e- 8       NA          NA
      187          193 <NA>          NA     NA    NA  4.83e-10       NA          NA
      188          194 <NA>          NA     NA    NA  2.97e-12       NA          NA
      189          195 <NA>          NA     NA    NA  1.02e-20       NA          NA
      190          196 <NA>          NA     NA    NA  7.91e-18       NA          NA
      191          197 <NA>          NA     NA    NA  3.58e-15       NA          NA
      192          198 <NA>          NA     NA    NA  9.46e-13       NA          NA
      193          199 <NA>          NA     NA    NA  1.46e-10       NA          NA
      194          200 <NA>          NA     NA    NA  1.31e- 8       NA          NA
      195          201 <NA>          NA     NA    NA  6.94e- 7       NA          NA
      196          202 <NA>          NA     NA    NA  2.15e- 5       NA          NA
      197          203 <NA>          NA     NA    NA  3.91e- 4       NA          NA
      198          204 <NA>          NA     NA    NA  4.19e- 3       NA          NA
      199          205 <NA>          NA     NA    NA  2.65e- 2       NA          NA
      200          206 <NA>          NA     NA    NA  9.92e- 2       NA          NA
      201          207 <NA>          NA     NA    NA  2.20e- 1       NA          NA
      202          208 <NA>          NA     NA    NA  2.88e- 1       NA          NA
      203          209 <NA>          NA     NA    NA  2.25e- 1       NA          NA
      204          210 <NA>          NA     NA    NA  1.04e- 1       NA          NA
      205          211 <NA>          NA     NA    NA  2.83e- 2       NA          NA
      206          212 <NA>          NA     NA    NA  4.58e- 3       NA          NA
      207          213 <NA>          NA     NA    NA  4.37e- 4       NA          NA
      208          214 <NA>          NA     NA    NA  2.46e- 5       NA          NA
      209          215 <NA>          NA     NA    NA  8.11e- 7       NA          NA
      210          216 <NA>          NA     NA    NA  1.57e- 8       NA          NA
      211          217 <NA>          NA     NA    NA  1.80e-10       NA          NA
      212          218 <NA>          NA     NA    NA  8.31e-21       NA          NA
      213          219 <NA>          NA     NA    NA  5.43e-18       NA          NA
      214          220 <NA>          NA     NA    NA  2.14e-15       NA          NA
      215          221 <NA>          NA     NA    NA  5.06e-13       NA          NA
      216          222 <NA>          NA     NA    NA  7.20e-11       NA          NA
      217          223 <NA>          NA     NA    NA  6.18e- 9       NA          NA
      218          224 <NA>          NA     NA    NA  3.20e- 7       NA          NA
      219          225 <NA>          NA     NA    NA  1.00e- 5       NA          NA
      220          226 <NA>          NA     NA    NA  1.91e- 4       NA          NA
      221          227 <NA>          NA     NA    NA  2.19e- 3       NA          NA
      222          228 <NA>          NA     NA    NA  1.53e- 2       NA          NA
      223          229 <NA>          NA     NA    NA  6.53e- 2       NA          NA
      224          230 <NA>          NA     NA    NA  1.69e- 1       NA          NA
      225          231 <NA>          NA     NA    NA  2.68e- 1       NA          NA
      226          232 <NA>          NA     NA    NA  2.59e- 1       NA          NA
      227          233 <NA>          NA     NA    NA  1.53e- 1       NA          NA
      228          234 <NA>          NA     NA    NA  5.48e- 2       NA          NA
      229          235 <NA>          NA     NA    NA  1.20e- 2       NA          NA
      230          236 <NA>          NA     NA    NA  1.60e- 3       NA          NA
      231          237 <NA>          NA     NA    NA  1.30e- 4       NA          NA
      232          238 <NA>          NA     NA    NA  6.37e- 6       NA          NA
      233          239 <NA>          NA     NA    NA  1.89e- 7       NA          NA
      234          240 <NA>          NA     NA    NA  3.44e- 9       NA          NA
      235          241 <NA>          NA     NA    NA  7.07e-21       NA          NA
      236          242 <NA>          NA     NA    NA  4.03e-18       NA          NA
      237          243 <NA>          NA     NA    NA  1.42e-15       NA          NA
      238          244 <NA>          NA     NA    NA  3.06e-13       NA          NA
      239          245 <NA>          NA     NA    NA  4.09e-11       NA          NA
      240          246 <NA>          NA     NA    NA  3.36e- 9       NA          NA
      241          247 <NA>          NA     NA    NA  1.71e- 7       NA          NA
      242          248 <NA>          NA     NA    NA  5.39e- 6       NA          NA
      243          249 <NA>          NA     NA    NA  1.05e- 4       NA          NA
      244          250 <NA>          NA     NA    NA  1.27e- 3       NA          NA
      245          251 <NA>          NA     NA    NA  9.57e- 3       NA          NA
      246          252 <NA>          NA     NA    NA  4.48e- 2       NA          NA
      247          253 <NA>          NA     NA    NA  1.31e- 1       NA          NA
      248          254 <NA>          NA     NA    NA  2.38e- 1       NA          NA
      249          255 <NA>          NA     NA    NA  2.71e- 1       NA          NA
      250          256 <NA>          NA     NA    NA  1.92e- 1       NA          NA
      251          257 <NA>          NA     NA    NA  8.49e- 2       NA          NA
      252          258 <NA>          NA     NA    NA  2.34e- 2       NA          NA
      253          259 <NA>          NA     NA    NA  4.02e- 3       NA          NA
      254          260 <NA>          NA     NA    NA  4.30e- 4       NA          NA
      255          261 <NA>          NA     NA    NA  2.85e- 5       NA          NA
      256          262 <NA>          NA     NA    NA  1.17e- 6       NA          NA
      257          263 <NA>          NA     NA    NA  3.04e- 8       NA          NA
      258          264 <NA>          NA     NA    NA  6.22e-21       NA          NA
      259          265 <NA>          NA     NA    NA  3.17e-18       NA          NA
      260          266 <NA>          NA     NA    NA  1.02e-15       NA          NA
      261          267 <NA>          NA     NA    NA  2.04e-13       NA          NA
      262          268 <NA>          NA     NA    NA  2.58e-11       NA          NA
      263          269 <NA>          NA     NA    NA  2.05e- 9       NA          NA
      264          270 <NA>          NA     NA    NA  1.03e- 7       NA          NA
      265          271 <NA>          NA     NA    NA  3.23e- 6       NA          NA
      266          272 <NA>          NA     NA    NA  6.43e- 5       NA          NA
      267          273 <NA>          NA     NA    NA  8.07e- 4       NA          NA
      268          274 <NA>          NA     NA    NA  6.41e- 3       NA          NA
      269          275 <NA>          NA     NA    NA  3.23e- 2       NA          NA
      270          276 <NA>          NA     NA    NA  1.03e- 1       NA          NA
      271          277 <NA>          NA     NA    NA  2.09e- 1       NA          NA
      272          278 <NA>          NA     NA    NA  2.69e- 1       NA          NA
      273          279 <NA>          NA     NA    NA  2.20e- 1       NA          NA
      274          280 <NA>          NA     NA    NA  1.14e- 1       NA          NA
      275          281 <NA>          NA     NA    NA  3.76e- 2       NA          NA
      276          282 <NA>          NA     NA    NA  7.85e- 3       NA          NA
      277          283 <NA>          NA     NA    NA  1.04e- 3       NA          NA
      278          284 <NA>          NA     NA    NA  8.70e- 5       NA          NA
      279          285 <NA>          NA     NA    NA  4.61e- 6       NA          NA
      280          286 <NA>          NA     NA    NA  1.57e- 7       NA          NA
      281          287 <NA>          NA     NA    NA -3.60e+ 0       NA          NA
      282          288 <NA>          NA     NA    NA -3.10e+ 0       NA          NA
      283          289 <NA>          NA     NA    NA -2.80e+ 0       NA          NA
      284          290 <NA>          NA     NA    NA -3.02e+ 0       NA          NA
      285          291 <NA>          NA     NA    NA -2.44e+ 0       NA          NA
      286          292 <NA>          NA     NA    NA -2.43e+ 0       NA          NA
      287          293 <NA>          NA     NA    NA -1.68e+ 0       NA          NA
      288          294 <NA>          NA     NA    NA -2.22e+ 0       NA          NA
      289          295 <NA>          NA     NA    NA -2.02e+ 0       NA          NA
      290          296 <NA>          NA     NA    NA -1.89e+ 0       NA          NA
      291          297 <NA>          NA     NA    NA -1.82e+ 0       NA          NA
      292          298 <NA>          NA     NA    NA -2.15e+ 0       NA          NA
      293          299 <NA>          NA     NA    NA -1.78e+ 0       NA          NA
      294          300 <NA>          NA     NA    NA -1.71e+ 0       NA          NA
      295          301 <NA>          NA     NA    NA -1.82e+ 0       NA          NA
      296          302 <NA>          NA     NA    NA -1.16e+ 0       NA          NA
      297          303 <NA>          NA     NA    NA -1.36e+ 0       NA          NA
      298          304 <NA>          NA     NA    NA -1.37e+ 0       NA          NA
      299          305 <NA>          NA     NA    NA -1.38e+ 0       NA          NA
      300          306 <NA>          NA     NA    NA -1.05e+ 0       NA          NA
      301          307 <NA>          NA     NA    NA -1.37e+ 0       NA          NA
      302          308 <NA>          NA     NA    NA -8.71e- 1       NA          NA
      303          309 <NA>          NA     NA    NA -1.06e+ 0       NA          NA
      304          310 <NA>          NA     NA    NA -1.07e+ 0       NA          NA
      305          311 <NA>          NA     NA    NA -1.16e+ 0       NA          NA
      306          312 <NA>          NA     NA    NA -1.18e+ 0       NA          NA
      307          313 <NA>          NA     NA    NA -8.40e- 1       NA          NA
      308          314 <NA>          NA     NA    NA -1.11e+ 0       NA          NA
      309          315 <NA>          NA     NA    NA -6.94e- 1       NA          NA
      310          352 <NA>          NA     NA    NA  1.5 e+ 0       NA          NA
      311          353 <NA>          NA     NA    NA  2   e+ 0       NA          NA
      312          354 <NA>          NA     NA    NA -1.49e+ 1       NA          NA
      313          355 <NA>          NA     NA    NA -2   e+ 2       NA          NA
      314          362 <NA>          NA     NA    NA  1.26e-16       NA          NA
      315          363 <NA>          NA     NA    NA  8.39e-11       NA          NA
      316          364 <NA>          NA     NA    NA  2.30e- 6       NA          NA
      317          365 <NA>          NA     NA    NA  2.74e- 3       NA          NA
      318          366 <NA>          NA     NA    NA  1.63e- 1       NA          NA
      319          367 <NA>          NA     NA    NA  6.32e- 1       NA          NA
      320          368 <NA>          NA     NA    NA  1.98e- 1       NA          NA
          estimation_type
          <chr>          
        1 constant       
        2 constant       
        3 constant       
        4 constant       
        5 constant       
        6 constant       
        7 constant       
        8 constant       
        9 constant       
       10 constant       
       11 constant       
       12 constant       
       13 constant       
       14 constant       
       15 constant       
       16 constant       
       17 constant       
       18 constant       
       19 constant       
       20 constant       
       21 constant       
       22 constant       
       23 constant       
       24 constant       
       25 constant       
       26 constant       
       27 constant       
       28 constant       
       29 constant       
       30 constant       
       31 constant       
       32 constant       
       33 constant       
       34 constant       
       35 constant       
       36 constant       
       37 constant       
       38 constant       
       39 constant       
       40 constant       
       41 constant       
       42 constant       
       43 constant       
       44 constant       
       45 constant       
       46 constant       
       47 constant       
       48 constant       
       49 constant       
       50 constant       
       51 constant       
       52 constant       
       53 constant       
       54 constant       
       55 constant       
       56 constant       
       57 constant       
       58 constant       
       59 constant       
       60 constant       
       61 constant       
       62 constant       
       63 constant       
       64 constant       
       65 constant       
       66 constant       
       67 constant       
       68 constant       
       69 constant       
       70 constant       
       71 constant       
       72 constant       
       73 constant       
       74 constant       
       75 constant       
       76 constant       
       77 constant       
       78 constant       
       79 constant       
       80 constant       
       81 constant       
       82 constant       
       83 constant       
       84 constant       
       85 constant       
       86 constant       
       87 constant       
       88 constant       
       89 constant       
       90 constant       
       91 constant       
       92 constant       
       93 constant       
       94 constant       
       95 constant       
       96 constant       
       97 constant       
       98 constant       
       99 constant       
      100 constant       
      101 constant       
      102 constant       
      103 constant       
      104 constant       
      105 constant       
      106 constant       
      107 constant       
      108 constant       
      109 constant       
      110 constant       
      111 constant       
      112 constant       
      113 constant       
      114 constant       
      115 constant       
      116 constant       
      117 constant       
      118 constant       
      119 constant       
      120 constant       
      121 constant       
      122 constant       
      123 constant       
      124 constant       
      125 constant       
      126 constant       
      127 constant       
      128 constant       
      129 constant       
      130 constant       
      131 constant       
      132 constant       
      133 constant       
      134 constant       
      135 constant       
      136 constant       
      137 constant       
      138 constant       
      139 constant       
      140 constant       
      141 constant       
      142 constant       
      143 constant       
      144 constant       
      145 constant       
      146 constant       
      147 constant       
      148 constant       
      149 constant       
      150 constant       
      151 constant       
      152 constant       
      153 constant       
      154 constant       
      155 constant       
      156 constant       
      157 constant       
      158 constant       
      159 constant       
      160 constant       
      161 constant       
      162 constant       
      163 constant       
      164 constant       
      165 constant       
      166 constant       
      167 constant       
      168 constant       
      169 constant       
      170 constant       
      171 constant       
      172 constant       
      173 constant       
      174 constant       
      175 constant       
      176 constant       
      177 constant       
      178 constant       
      179 constant       
      180 constant       
      181 constant       
      182 constant       
      183 constant       
      184 constant       
      185 constant       
      186 constant       
      187 constant       
      188 constant       
      189 constant       
      190 constant       
      191 constant       
      192 constant       
      193 constant       
      194 constant       
      195 constant       
      196 constant       
      197 constant       
      198 constant       
      199 constant       
      200 constant       
      201 constant       
      202 constant       
      203 constant       
      204 constant       
      205 constant       
      206 constant       
      207 constant       
      208 constant       
      209 constant       
      210 constant       
      211 constant       
      212 constant       
      213 constant       
      214 constant       
      215 constant       
      216 constant       
      217 constant       
      218 constant       
      219 constant       
      220 constant       
      221 constant       
      222 constant       
      223 constant       
      224 constant       
      225 constant       
      226 constant       
      227 constant       
      228 constant       
      229 constant       
      230 constant       
      231 constant       
      232 constant       
      233 constant       
      234 constant       
      235 constant       
      236 constant       
      237 constant       
      238 constant       
      239 constant       
      240 constant       
      241 constant       
      242 constant       
      243 constant       
      244 constant       
      245 constant       
      246 constant       
      247 constant       
      248 constant       
      249 constant       
      250 constant       
      251 constant       
      252 constant       
      253 constant       
      254 constant       
      255 constant       
      256 constant       
      257 constant       
      258 constant       
      259 constant       
      260 constant       
      261 constant       
      262 constant       
      263 constant       
      264 constant       
      265 constant       
      266 constant       
      267 constant       
      268 constant       
      269 constant       
      270 constant       
      271 constant       
      272 constant       
      273 constant       
      274 constant       
      275 constant       
      276 constant       
      277 constant       
      278 constant       
      279 constant       
      280 constant       
      281 constant       
      282 constant       
      283 constant       
      284 constant       
      285 constant       
      286 constant       
      287 constant       
      288 constant       
      289 constant       
      290 constant       
      291 constant       
      292 constant       
      293 constant       
      294 constant       
      295 constant       
      296 constant       
      297 constant       
      298 constant       
      299 constant       
      300 constant       
      301 constant       
      302 constant       
      303 constant       
      304 constant       
      305 constant       
      306 constant       
      307 constant       
      308 constant       
      309 constant       
      310 constant       
      311 constant       
      312 constant       
      313 constant       
      314 constant       
      315 constant       
      316 constant       
      317 constant       
      318 constant       
      319 constant       
      320 constant       
      # i 11,445 more rows

# get_estimates() works with estimation run

    Code
      print(dplyr::select(get_estimates(readRDS(fit_files[[1]])), -estimate,
      -uncertainty, -gradient), n = 320, width = Inf)
    Output
      # A tibble: 20,347 x 15
      # Groups:   label [91]
          module_name module_id module_type label                    type   type_id
          <chr>           <int> <chr>       <chr>                    <chr>    <int>
        1 selectivity         1 Logistic    inflection_point         vector       1
        2 selectivity         1 Logistic    slope                    vector       2
        3 Fleet               1 fleet       log_q                    vector       3
        4 Fleet               1 fleet       log_Fmort                vector       4
        5 Fleet               1 fleet       age_to_length_conversion vector      11
        6 Fleet               1 fleet       age_to_length_conversion vector      11
        7 Fleet               1 fleet       age_to_length_conversion vector      11
        8 Fleet               1 fleet       age_to_length_conversion vector      11
        9 Fleet               1 fleet       age_to_length_conversion vector      11
       10 Fleet               1 fleet       age_to_length_conversion vector      11
       11 Fleet               1 fleet       age_to_length_conversion vector      11
       12 Fleet               1 fleet       age_to_length_conversion vector      11
       13 Fleet               1 fleet       age_to_length_conversion vector      11
       14 Fleet               1 fleet       age_to_length_conversion vector      11
       15 Fleet               1 fleet       age_to_length_conversion vector      11
       16 Fleet               1 fleet       age_to_length_conversion vector      11
       17 Fleet               1 fleet       age_to_length_conversion vector      11
       18 Fleet               1 fleet       age_to_length_conversion vector      11
       19 Fleet               1 fleet       age_to_length_conversion vector      11
       20 Fleet               1 fleet       age_to_length_conversion vector      11
       21 Fleet               1 fleet       age_to_length_conversion vector      11
       22 Fleet               1 fleet       age_to_length_conversion vector      11
       23 Fleet               1 fleet       age_to_length_conversion vector      11
       24 Fleet               1 fleet       age_to_length_conversion vector      11
       25 Fleet               1 fleet       age_to_length_conversion vector      11
       26 Fleet               1 fleet       age_to_length_conversion vector      11
       27 Fleet               1 fleet       age_to_length_conversion vector      11
       28 Fleet               1 fleet       age_to_length_conversion vector      11
       29 Fleet               1 fleet       age_to_length_conversion vector      11
       30 Fleet               1 fleet       age_to_length_conversion vector      11
       31 Fleet               1 fleet       age_to_length_conversion vector      11
       32 Fleet               1 fleet       age_to_length_conversion vector      11
       33 Fleet               1 fleet       age_to_length_conversion vector      11
       34 Fleet               1 fleet       age_to_length_conversion vector      11
       35 Fleet               1 fleet       age_to_length_conversion vector      11
       36 Fleet               1 fleet       age_to_length_conversion vector      11
       37 Fleet               1 fleet       age_to_length_conversion vector      11
       38 Fleet               1 fleet       age_to_length_conversion vector      11
       39 Fleet               1 fleet       age_to_length_conversion vector      11
       40 Fleet               1 fleet       age_to_length_conversion vector      11
       41 Fleet               1 fleet       age_to_length_conversion vector      11
       42 Fleet               1 fleet       age_to_length_conversion vector      11
       43 Fleet               1 fleet       age_to_length_conversion vector      11
       44 Fleet               1 fleet       age_to_length_conversion vector      11
       45 Fleet               1 fleet       age_to_length_conversion vector      11
       46 Fleet               1 fleet       age_to_length_conversion vector      11
       47 Fleet               1 fleet       age_to_length_conversion vector      11
       48 Fleet               1 fleet       age_to_length_conversion vector      11
       49 Fleet               1 fleet       age_to_length_conversion vector      11
       50 Fleet               1 fleet       age_to_length_conversion vector      11
       51 Fleet               1 fleet       age_to_length_conversion vector      11
       52 Fleet               1 fleet       age_to_length_conversion vector      11
       53 Fleet               1 fleet       age_to_length_conversion vector      11
       54 Fleet               1 fleet       age_to_length_conversion vector      11
       55 Fleet               1 fleet       age_to_length_conversion vector      11
       56 Fleet               1 fleet       age_to_length_conversion vector      11
       57 Fleet               1 fleet       age_to_length_conversion vector      11
       58 Fleet               1 fleet       age_to_length_conversion vector      11
       59 Fleet               1 fleet       age_to_length_conversion vector      11
       60 Fleet               1 fleet       age_to_length_conversion vector      11
       61 Fleet               1 fleet       age_to_length_conversion vector      11
       62 Fleet               1 fleet       age_to_length_conversion vector      11
       63 Fleet               1 fleet       age_to_length_conversion vector      11
       64 Fleet               1 fleet       age_to_length_conversion vector      11
       65 Fleet               1 fleet       age_to_length_conversion vector      11
       66 Fleet               1 fleet       age_to_length_conversion vector      11
       67 Fleet               1 fleet       age_to_length_conversion vector      11
       68 Fleet               1 fleet       age_to_length_conversion vector      11
       69 Fleet               1 fleet       age_to_length_conversion vector      11
       70 Fleet               1 fleet       age_to_length_conversion vector      11
       71 Fleet               1 fleet       age_to_length_conversion vector      11
       72 Fleet               1 fleet       age_to_length_conversion vector      11
       73 Fleet               1 fleet       age_to_length_conversion vector      11
       74 Fleet               1 fleet       age_to_length_conversion vector      11
       75 Fleet               1 fleet       age_to_length_conversion vector      11
       76 Fleet               1 fleet       age_to_length_conversion vector      11
       77 Fleet               1 fleet       age_to_length_conversion vector      11
       78 Fleet               1 fleet       age_to_length_conversion vector      11
       79 Fleet               1 fleet       age_to_length_conversion vector      11
       80 Fleet               1 fleet       age_to_length_conversion vector      11
       81 Fleet               1 fleet       age_to_length_conversion vector      11
       82 Fleet               1 fleet       age_to_length_conversion vector      11
       83 Fleet               1 fleet       age_to_length_conversion vector      11
       84 Fleet               1 fleet       age_to_length_conversion vector      11
       85 Fleet               1 fleet       age_to_length_conversion vector      11
       86 Fleet               1 fleet       age_to_length_conversion vector      11
       87 Fleet               1 fleet       age_to_length_conversion vector      11
       88 Fleet               1 fleet       age_to_length_conversion vector      11
       89 Fleet               1 fleet       age_to_length_conversion vector      11
       90 Fleet               1 fleet       age_to_length_conversion vector      11
       91 Fleet               1 fleet       age_to_length_conversion vector      11
       92 Fleet               1 fleet       age_to_length_conversion vector      11
       93 Fleet               1 fleet       age_to_length_conversion vector      11
       94 Fleet               1 fleet       age_to_length_conversion vector      11
       95 Fleet               1 fleet       age_to_length_conversion vector      11
       96 Fleet               1 fleet       age_to_length_conversion vector      11
       97 Fleet               1 fleet       age_to_length_conversion vector      11
       98 Fleet               1 fleet       age_to_length_conversion vector      11
       99 Fleet               1 fleet       age_to_length_conversion vector      11
      100 Fleet               1 fleet       age_to_length_conversion vector      11
      101 Fleet               1 fleet       age_to_length_conversion vector      11
      102 Fleet               1 fleet       age_to_length_conversion vector      11
      103 Fleet               1 fleet       age_to_length_conversion vector      11
      104 Fleet               1 fleet       age_to_length_conversion vector      11
      105 Fleet               1 fleet       age_to_length_conversion vector      11
      106 Fleet               1 fleet       age_to_length_conversion vector      11
      107 Fleet               1 fleet       age_to_length_conversion vector      11
      108 Fleet               1 fleet       age_to_length_conversion vector      11
      109 Fleet               1 fleet       age_to_length_conversion vector      11
      110 Fleet               1 fleet       age_to_length_conversion vector      11
      111 Fleet               1 fleet       age_to_length_conversion vector      11
      112 Fleet               1 fleet       age_to_length_conversion vector      11
      113 Fleet               1 fleet       age_to_length_conversion vector      11
      114 Fleet               1 fleet       age_to_length_conversion vector      11
      115 Fleet               1 fleet       age_to_length_conversion vector      11
      116 Fleet               1 fleet       age_to_length_conversion vector      11
      117 Fleet               1 fleet       age_to_length_conversion vector      11
      118 Fleet               1 fleet       age_to_length_conversion vector      11
      119 Fleet               1 fleet       age_to_length_conversion vector      11
      120 Fleet               1 fleet       age_to_length_conversion vector      11
      121 Fleet               1 fleet       age_to_length_conversion vector      11
      122 Fleet               1 fleet       age_to_length_conversion vector      11
      123 Fleet               1 fleet       age_to_length_conversion vector      11
      124 Fleet               1 fleet       age_to_length_conversion vector      11
      125 Fleet               1 fleet       age_to_length_conversion vector      11
      126 Fleet               1 fleet       age_to_length_conversion vector      11
      127 Fleet               1 fleet       age_to_length_conversion vector      11
      128 Fleet               1 fleet       age_to_length_conversion vector      11
      129 Fleet               1 fleet       age_to_length_conversion vector      11
      130 Fleet               1 fleet       age_to_length_conversion vector      11
      131 Fleet               1 fleet       age_to_length_conversion vector      11
      132 Fleet               1 fleet       age_to_length_conversion vector      11
      133 Fleet               1 fleet       age_to_length_conversion vector      11
      134 Fleet               1 fleet       age_to_length_conversion vector      11
      135 Fleet               1 fleet       age_to_length_conversion vector      11
      136 Fleet               1 fleet       age_to_length_conversion vector      11
      137 Fleet               1 fleet       age_to_length_conversion vector      11
      138 Fleet               1 fleet       age_to_length_conversion vector      11
      139 Fleet               1 fleet       age_to_length_conversion vector      11
      140 Fleet               1 fleet       age_to_length_conversion vector      11
      141 Fleet               1 fleet       age_to_length_conversion vector      11
      142 Fleet               1 fleet       age_to_length_conversion vector      11
      143 Fleet               1 fleet       age_to_length_conversion vector      11
      144 Fleet               1 fleet       age_to_length_conversion vector      11
      145 Fleet               1 fleet       age_to_length_conversion vector      11
      146 Fleet               1 fleet       age_to_length_conversion vector      11
      147 Fleet               1 fleet       age_to_length_conversion vector      11
      148 Fleet               1 fleet       age_to_length_conversion vector      11
      149 Fleet               1 fleet       age_to_length_conversion vector      11
      150 Fleet               1 fleet       age_to_length_conversion vector      11
      151 Fleet               1 fleet       age_to_length_conversion vector      11
      152 Fleet               1 fleet       age_to_length_conversion vector      11
      153 Fleet               1 fleet       age_to_length_conversion vector      11
      154 Fleet               1 fleet       age_to_length_conversion vector      11
      155 Fleet               1 fleet       age_to_length_conversion vector      11
      156 Fleet               1 fleet       age_to_length_conversion vector      11
      157 Fleet               1 fleet       age_to_length_conversion vector      11
      158 Fleet               1 fleet       age_to_length_conversion vector      11
      159 Fleet               1 fleet       age_to_length_conversion vector      11
      160 Fleet               1 fleet       age_to_length_conversion vector      11
      161 Fleet               1 fleet       age_to_length_conversion vector      11
      162 Fleet               1 fleet       age_to_length_conversion vector      11
      163 Fleet               1 fleet       age_to_length_conversion vector      11
      164 Fleet               1 fleet       age_to_length_conversion vector      11
      165 Fleet               1 fleet       age_to_length_conversion vector      11
      166 Fleet               1 fleet       age_to_length_conversion vector      11
      167 Fleet               1 fleet       age_to_length_conversion vector      11
      168 Fleet               1 fleet       age_to_length_conversion vector      11
      169 Fleet               1 fleet       age_to_length_conversion vector      11
      170 Fleet               1 fleet       age_to_length_conversion vector      11
      171 Fleet               1 fleet       age_to_length_conversion vector      11
      172 Fleet               1 fleet       age_to_length_conversion vector      11
      173 Fleet               1 fleet       age_to_length_conversion vector      11
      174 Fleet               1 fleet       age_to_length_conversion vector      11
      175 Fleet               1 fleet       age_to_length_conversion vector      11
      176 Fleet               1 fleet       age_to_length_conversion vector      11
      177 Fleet               1 fleet       age_to_length_conversion vector      11
      178 Fleet               1 fleet       age_to_length_conversion vector      11
      179 Fleet               1 fleet       age_to_length_conversion vector      11
      180 Fleet               1 fleet       age_to_length_conversion vector      11
      181 Fleet               1 fleet       age_to_length_conversion vector      11
      182 Fleet               1 fleet       age_to_length_conversion vector      11
      183 Fleet               1 fleet       age_to_length_conversion vector      11
      184 Fleet               1 fleet       age_to_length_conversion vector      11
      185 Fleet               1 fleet       age_to_length_conversion vector      11
      186 Fleet               1 fleet       age_to_length_conversion vector      11
      187 Fleet               1 fleet       age_to_length_conversion vector      11
      188 Fleet               1 fleet       age_to_length_conversion vector      11
      189 Fleet               1 fleet       age_to_length_conversion vector      11
      190 Fleet               1 fleet       age_to_length_conversion vector      11
      191 Fleet               1 fleet       age_to_length_conversion vector      11
      192 Fleet               1 fleet       age_to_length_conversion vector      11
      193 Fleet               1 fleet       age_to_length_conversion vector      11
      194 Fleet               1 fleet       age_to_length_conversion vector      11
      195 Fleet               1 fleet       age_to_length_conversion vector      11
      196 Fleet               1 fleet       age_to_length_conversion vector      11
      197 Fleet               1 fleet       age_to_length_conversion vector      11
      198 Fleet               1 fleet       age_to_length_conversion vector      11
      199 Fleet               1 fleet       age_to_length_conversion vector      11
      200 Fleet               1 fleet       age_to_length_conversion vector      11
      201 Fleet               1 fleet       age_to_length_conversion vector      11
      202 Fleet               1 fleet       age_to_length_conversion vector      11
      203 Fleet               1 fleet       age_to_length_conversion vector      11
      204 Fleet               1 fleet       age_to_length_conversion vector      11
      205 Fleet               1 fleet       age_to_length_conversion vector      11
      206 Fleet               1 fleet       age_to_length_conversion vector      11
      207 Fleet               1 fleet       age_to_length_conversion vector      11
      208 Fleet               1 fleet       age_to_length_conversion vector      11
      209 Fleet               1 fleet       age_to_length_conversion vector      11
      210 Fleet               1 fleet       age_to_length_conversion vector      11
      211 Fleet               1 fleet       age_to_length_conversion vector      11
      212 Fleet               1 fleet       age_to_length_conversion vector      11
      213 Fleet               1 fleet       age_to_length_conversion vector      11
      214 Fleet               1 fleet       age_to_length_conversion vector      11
      215 Fleet               1 fleet       age_to_length_conversion vector      11
      216 Fleet               1 fleet       age_to_length_conversion vector      11
      217 Fleet               1 fleet       age_to_length_conversion vector      11
      218 Fleet               1 fleet       age_to_length_conversion vector      11
      219 Fleet               1 fleet       age_to_length_conversion vector      11
      220 Fleet               1 fleet       age_to_length_conversion vector      11
      221 Fleet               1 fleet       age_to_length_conversion vector      11
      222 Fleet               1 fleet       age_to_length_conversion vector      11
      223 Fleet               1 fleet       age_to_length_conversion vector      11
      224 Fleet               1 fleet       age_to_length_conversion vector      11
      225 Fleet               1 fleet       age_to_length_conversion vector      11
      226 Fleet               1 fleet       age_to_length_conversion vector      11
      227 Fleet               1 fleet       age_to_length_conversion vector      11
      228 Fleet               1 fleet       age_to_length_conversion vector      11
      229 Fleet               1 fleet       age_to_length_conversion vector      11
      230 Fleet               1 fleet       age_to_length_conversion vector      11
      231 Fleet               1 fleet       age_to_length_conversion vector      11
      232 Fleet               1 fleet       age_to_length_conversion vector      11
      233 Fleet               1 fleet       age_to_length_conversion vector      11
      234 Fleet               1 fleet       age_to_length_conversion vector      11
      235 Fleet               1 fleet       age_to_length_conversion vector      11
      236 Fleet               1 fleet       age_to_length_conversion vector      11
      237 Fleet               1 fleet       age_to_length_conversion vector      11
      238 Fleet               1 fleet       age_to_length_conversion vector      11
      239 Fleet               1 fleet       age_to_length_conversion vector      11
      240 Fleet               1 fleet       age_to_length_conversion vector      11
      241 Fleet               1 fleet       age_to_length_conversion vector      11
      242 Fleet               1 fleet       age_to_length_conversion vector      11
      243 Fleet               1 fleet       age_to_length_conversion vector      11
      244 Fleet               1 fleet       age_to_length_conversion vector      11
      245 Fleet               1 fleet       age_to_length_conversion vector      11
      246 Fleet               1 fleet       age_to_length_conversion vector      11
      247 Fleet               1 fleet       age_to_length_conversion vector      11
      248 Fleet               1 fleet       age_to_length_conversion vector      11
      249 Fleet               1 fleet       age_to_length_conversion vector      11
      250 Fleet               1 fleet       age_to_length_conversion vector      11
      251 Fleet               1 fleet       age_to_length_conversion vector      11
      252 Fleet               1 fleet       age_to_length_conversion vector      11
      253 Fleet               1 fleet       age_to_length_conversion vector      11
      254 Fleet               1 fleet       age_to_length_conversion vector      11
      255 Fleet               1 fleet       age_to_length_conversion vector      11
      256 Fleet               1 fleet       age_to_length_conversion vector      11
      257 Fleet               1 fleet       age_to_length_conversion vector      11
      258 Fleet               1 fleet       age_to_length_conversion vector      11
      259 Fleet               1 fleet       age_to_length_conversion vector      11
      260 Fleet               1 fleet       age_to_length_conversion vector      11
      261 Fleet               1 fleet       age_to_length_conversion vector      11
      262 Fleet               1 fleet       age_to_length_conversion vector      11
      263 Fleet               1 fleet       age_to_length_conversion vector      11
      264 Fleet               1 fleet       age_to_length_conversion vector      11
      265 Fleet               1 fleet       age_to_length_conversion vector      11
      266 Fleet               1 fleet       age_to_length_conversion vector      11
      267 Fleet               1 fleet       age_to_length_conversion vector      11
      268 Fleet               1 fleet       age_to_length_conversion vector      11
      269 Fleet               1 fleet       age_to_length_conversion vector      11
      270 Fleet               1 fleet       age_to_length_conversion vector      11
      271 Fleet               1 fleet       age_to_length_conversion vector      11
      272 Fleet               1 fleet       age_to_length_conversion vector      11
      273 Fleet               1 fleet       age_to_length_conversion vector      11
      274 Fleet               1 fleet       age_to_length_conversion vector      11
      275 Fleet               1 fleet       age_to_length_conversion vector      11
      276 Fleet               1 fleet       age_to_length_conversion vector      11
      277 Fleet               1 fleet       age_to_length_conversion vector      11
      278 Fleet               1 fleet       age_to_length_conversion vector      11
      279 Fleet               1 fleet       age_to_length_conversion vector      11
      280 Fleet               1 fleet       age_to_length_conversion vector      11
      281 Fleet               1 fleet       log_Fmort                vector       4
      282 Fleet               1 fleet       log_Fmort                vector       4
      283 Fleet               1 fleet       log_Fmort                vector       4
      284 Fleet               1 fleet       log_Fmort                vector       4
      285 Fleet               1 fleet       log_Fmort                vector       4
      286 Fleet               1 fleet       log_Fmort                vector       4
      287 Fleet               1 fleet       log_Fmort                vector       4
      288 Fleet               1 fleet       log_Fmort                vector       4
      289 Fleet               1 fleet       log_Fmort                vector       4
      290 Fleet               1 fleet       log_Fmort                vector       4
      291 Fleet               1 fleet       log_Fmort                vector       4
      292 Fleet               1 fleet       log_Fmort                vector       4
      293 Fleet               1 fleet       log_Fmort                vector       4
      294 Fleet               1 fleet       log_Fmort                vector       4
      295 Fleet               1 fleet       log_Fmort                vector       4
      296 Fleet               1 fleet       log_Fmort                vector       4
      297 Fleet               1 fleet       log_Fmort                vector       4
      298 Fleet               1 fleet       log_Fmort                vector       4
      299 Fleet               1 fleet       log_Fmort                vector       4
      300 Fleet               1 fleet       log_Fmort                vector       4
      301 Fleet               1 fleet       log_Fmort                vector       4
      302 Fleet               1 fleet       log_Fmort                vector       4
      303 Fleet               1 fleet       log_Fmort                vector       4
      304 Fleet               1 fleet       log_Fmort                vector       4
      305 Fleet               1 fleet       log_Fmort                vector       4
      306 Fleet               1 fleet       log_Fmort                vector       4
      307 Fleet               1 fleet       log_Fmort                vector       4
      308 Fleet               1 fleet       log_Fmort                vector       4
      309 Fleet               1 fleet       log_Fmort                vector       4
      310 selectivity         2 Logistic    inflection_point         vector      19
      311 selectivity         2 Logistic    slope                    vector      20
      312 Fleet               2 fleet       log_q                    vector      21
      313 Fleet               2 fleet       log_Fmort                vector      22
      314 Fleet               2 fleet       age_to_length_conversion vector      29
      315 Fleet               2 fleet       age_to_length_conversion vector      29
      316 Fleet               2 fleet       age_to_length_conversion vector      29
      317 Fleet               2 fleet       age_to_length_conversion vector      29
      318 Fleet               2 fleet       age_to_length_conversion vector      29
      319 Fleet               2 fleet       age_to_length_conversion vector      29
      320 Fleet               2 fleet       age_to_length_conversion vector      29
          parameter_id fleet_name   age length  time   initial log_like log_like_cv
                 <int> <chr>      <dbl>  <dbl> <int>     <dbl>    <dbl>       <dbl>
        1            1 <NA>          NA     NA    NA NA              NA          NA
        2            2 <NA>          NA     NA    NA NA              NA          NA
        3            3 <NA>          NA     NA    NA  0              NA          NA
        4            4 <NA>          NA     NA    NA NA              NA          NA
        5           11 <NA>          NA     NA    NA  1.26e-16       NA          NA
        6           12 <NA>          NA     NA    NA  8.39e-11       NA          NA
        7           13 <NA>          NA     NA    NA  2.30e- 6       NA          NA
        8           14 <NA>          NA     NA    NA  2.74e- 3       NA          NA
        9           15 <NA>          NA     NA    NA  1.63e- 1       NA          NA
       10           16 <NA>          NA     NA    NA  6.32e- 1       NA          NA
       11           17 <NA>          NA     NA    NA  1.98e- 1       NA          NA
       12           18 <NA>          NA     NA    NA  4.13e- 3       NA          NA
       13           19 <NA>          NA     NA    NA  4.36e- 6       NA          NA
       14           20 <NA>          NA     NA    NA  2.02e-10       NA          NA
       15           21 <NA>          NA     NA    NA  3.33e-16       NA          NA
       16           22 <NA>          NA     NA    NA  0              NA          NA
       17           23 <NA>          NA     NA    NA  0              NA          NA
       18           24 <NA>          NA     NA    NA  0              NA          NA
       19           25 <NA>          NA     NA    NA  0              NA          NA
       20           26 <NA>          NA     NA    NA  0              NA          NA
       21           27 <NA>          NA     NA    NA  0              NA          NA
       22           28 <NA>          NA     NA    NA  0              NA          NA
       23           29 <NA>          NA     NA    NA  0              NA          NA
       24           30 <NA>          NA     NA    NA  0              NA          NA
       25           31 <NA>          NA     NA    NA  0              NA          NA
       26           32 <NA>          NA     NA    NA  0              NA          NA
       27           33 <NA>          NA     NA    NA  0              NA          NA
       28           34 <NA>          NA     NA    NA  3.27e-18       NA          NA
       29           35 <NA>          NA     NA    NA  2.15e-13       NA          NA
       30           36 <NA>          NA     NA    NA  2.20e- 9       NA          NA
       31           37 <NA>          NA     NA    NA  3.54e- 6       NA          NA
       32           38 <NA>          NA     NA    NA  9.19e- 4       NA          NA
       33           39 <NA>          NA     NA    NA  4.03e- 2       NA          NA
       34           40 <NA>          NA     NA    NA  3.18e- 1       NA          NA
       35           41 <NA>          NA     NA    NA  4.86e- 1       NA          NA
       36           42 <NA>          NA     NA    NA  1.46e- 1       NA          NA
       37           43 <NA>          NA     NA    NA  8.23e- 3       NA          NA
       38           44 <NA>          NA     NA    NA  8.08e- 5       NA          NA
       39           45 <NA>          NA     NA    NA  1.31e- 7       NA          NA
       40           46 <NA>          NA     NA    NA  3.37e-11       NA          NA
       41           47 <NA>          NA     NA    NA  1.33e-15       NA          NA
       42           48 <NA>          NA     NA    NA  0              NA          NA
       43           49 <NA>          NA     NA    NA  0              NA          NA
       44           50 <NA>          NA     NA    NA  0              NA          NA
       45           51 <NA>          NA     NA    NA  0              NA          NA
       46           52 <NA>          NA     NA    NA  0              NA          NA
       47           53 <NA>          NA     NA    NA  0              NA          NA
       48           54 <NA>          NA     NA    NA  0              NA          NA
       49           55 <NA>          NA     NA    NA  0              NA          NA
       50           56 <NA>          NA     NA    NA  0              NA          NA
       51           57 <NA>          NA     NA    NA  4.35e-19       NA          NA
       52           58 <NA>          NA     NA    NA  6.73e-15       NA          NA
       53           59 <NA>          NA     NA    NA  2.84e-11       NA          NA
       54           60 <NA>          NA     NA    NA  3.28e- 8       NA          NA
       55           61 <NA>          NA     NA    NA  1.05e- 5       NA          NA
       56           62 <NA>          NA     NA    NA  9.44e- 4       NA          NA
       57           63 <NA>          NA     NA    NA  2.44e- 2       NA          NA
       58           64 <NA>          NA     NA    NA  1.85e- 1       NA          NA
       59           65 <NA>          NA     NA    NA  4.24e- 1       NA          NA
       60           66 <NA>          NA     NA    NA  2.98e- 1       NA          NA
       61           67 <NA>          NA     NA    NA  6.36e- 2       NA          NA
       62           68 <NA>          NA     NA    NA  4.04e- 3       NA          NA
       63           69 <NA>          NA     NA    NA  7.43e- 5       NA          NA
       64           70 <NA>          NA     NA    NA  3.87e- 7       NA          NA
       65           71 <NA>          NA     NA    NA  5.62e-10       NA          NA
       66           72 <NA>          NA     NA    NA  2.24e-13       NA          NA
       67           73 <NA>          NA     NA    NA  0              NA          NA
       68           74 <NA>          NA     NA    NA  0              NA          NA
       69           75 <NA>          NA     NA    NA  0              NA          NA
       70           76 <NA>          NA     NA    NA  0              NA          NA
       71           77 <NA>          NA     NA    NA  0              NA          NA
       72           78 <NA>          NA     NA    NA  0              NA          NA
       73           79 <NA>          NA     NA    NA  0              NA          NA
       74           80 <NA>          NA     NA    NA  1.23e-19       NA          NA
       75           81 <NA>          NA     NA    NA  7.29e-16       NA          NA
       76           82 <NA>          NA     NA    NA  1.58e-12       NA          NA
       77           83 <NA>          NA     NA    NA  1.25e- 9       NA          NA
       78           84 <NA>          NA     NA    NA  3.68e- 7       NA          NA
       79           85 <NA>          NA     NA    NA  4.02e- 5       NA          NA
       80           86 <NA>          NA     NA    NA  1.65e- 3       NA          NA
       81           87 <NA>          NA     NA    NA  2.56e- 2       NA          NA
       82           88 <NA>          NA     NA    NA  1.54e- 1       NA          NA
       83           89 <NA>          NA     NA    NA  3.58e- 1       NA          NA
       84           90 <NA>          NA     NA    NA  3.27e- 1       NA          NA
       85           91 <NA>          NA     NA    NA  1.17e- 1       NA          NA
       86           92 <NA>          NA     NA    NA  1.62e- 2       NA          NA
       87           93 <NA>          NA     NA    NA  8.65e- 4       NA          NA
       88           94 <NA>          NA     NA    NA  1.75e- 5       NA          NA
       89           95 <NA>          NA     NA    NA  1.32e- 7       NA          NA
       90           96 <NA>          NA     NA    NA  3.71e-10       NA          NA
       91           97 <NA>          NA     NA    NA  3.85e-13       NA          NA
       92           98 <NA>          NA     NA    NA  1.11e-16       NA          NA
       93           99 <NA>          NA     NA    NA  0              NA          NA
       94          100 <NA>          NA     NA    NA  0              NA          NA
       95          101 <NA>          NA     NA    NA  0              NA          NA
       96          102 <NA>          NA     NA    NA  0              NA          NA
       97          103 <NA>          NA     NA    NA  5.27e-20       NA          NA
       98          104 <NA>          NA     NA    NA  1.59e-16       NA          NA
       99          105 <NA>          NA     NA    NA  2.09e-13       NA          NA
      100          106 <NA>          NA     NA    NA  1.20e-10       NA          NA
      101          107 <NA>          NA     NA    NA  3.04e- 8       NA          NA
      102          108 <NA>          NA     NA    NA  3.38e- 6       NA          NA
      103          109 <NA>          NA     NA    NA  1.67e- 4       NA          NA
      104          110 <NA>          NA     NA    NA  3.67e- 3       NA          NA
      105          111 <NA>          NA     NA    NA  3.63e- 2       NA          NA
      106          112 <NA>          NA     NA    NA  1.63e- 1       NA          NA
      107          113 <NA>          NA     NA    NA  3.31e- 1       NA          NA
      108          114 <NA>          NA     NA    NA  3.08e- 1       NA          NA
      109          115 <NA>          NA     NA    NA  1.31e- 1       NA          NA
      110          116 <NA>          NA     NA    NA  2.52e- 2       NA          NA
      111          117 <NA>          NA     NA    NA  2.20e- 3       NA          NA
      112          118 <NA>          NA     NA    NA  8.62e- 5       NA          NA
      113          119 <NA>          NA     NA    NA  1.50e- 6       NA          NA
      114          120 <NA>          NA     NA    NA  1.16e- 8       NA          NA
      115          121 <NA>          NA     NA    NA  3.94e-11       NA          NA
      116          122 <NA>          NA     NA    NA  5.88e-14       NA          NA
      117          123 <NA>          NA     NA    NA  0              NA          NA
      118          124 <NA>          NA     NA    NA  0              NA          NA
      119          125 <NA>          NA     NA    NA  0              NA          NA
      120          126 <NA>          NA     NA    NA  2.88e-20       NA          NA
      121          127 <NA>          NA     NA    NA  5.32e-17       NA          NA
      122          128 <NA>          NA     NA    NA  4.81e-14       NA          NA
      123          129 <NA>          NA     NA    NA  2.13e-11       NA          NA
      124          130 <NA>          NA     NA    NA  4.62e- 9       NA          NA
      125          131 <NA>          NA     NA    NA  4.94e- 7       NA          NA
      126          132 <NA>          NA     NA    NA  2.61e- 5       NA          NA
      127          133 <NA>          NA     NA    NA  6.86e- 4       NA          NA
      128          134 <NA>          NA     NA    NA  8.98e- 3       NA          NA
      129          135 <NA>          NA     NA    NA  5.89e- 2       NA          NA
      130          136 <NA>          NA     NA    NA  1.94e- 1       NA          NA
      131          137 <NA>          NA     NA    NA  3.23e- 1       NA          NA
      132          138 <NA>          NA     NA    NA  2.72e- 1       NA          NA
      133          139 <NA>          NA     NA    NA  1.15e- 1       NA          NA
      134          140 <NA>          NA     NA    NA  2.47e- 2       NA          NA
      135          141 <NA>          NA     NA    NA  2.66e- 3       NA          NA
      136          142 <NA>          NA     NA    NA  1.43e- 4       NA          NA
      137          143 <NA>          NA     NA    NA  3.81e- 6       NA          NA
      138          144 <NA>          NA     NA    NA  5.04e- 8       NA          NA
      139          145 <NA>          NA     NA    NA  3.29e-10       NA          NA
      140          146 <NA>          NA     NA    NA  1.06e-12       NA          NA
      141          147 <NA>          NA     NA    NA  1.67e-15       NA          NA
      142          148 <NA>          NA     NA    NA  0              NA          NA
      143          149 <NA>          NA     NA    NA  1.85e-20       NA          NA
      144          150 <NA>          NA     NA    NA  2.36e-17       NA          NA
      145          151 <NA>          NA     NA    NA  1.60e-14       NA          NA
      146          152 <NA>          NA     NA    NA  5.73e-12       NA          NA
      147          153 <NA>          NA     NA    NA  1.09e- 9       NA          NA
      148          154 <NA>          NA     NA    NA  1.10e- 7       NA          NA
      149          155 <NA>          NA     NA    NA  5.93e- 6       NA          NA
      150          156 <NA>          NA     NA    NA  1.71e- 4       NA          NA
      151          157 <NA>          NA     NA    NA  2.63e- 3       NA          NA
      152          158 <NA>          NA     NA    NA  2.18e- 2       NA          NA
      153          159 <NA>          NA     NA    NA  9.79e- 2       NA          NA
      154          160 <NA>          NA     NA    NA  2.37e- 1       NA          NA
      155          161 <NA>          NA     NA    NA  3.12e- 1       NA          NA
      156          162 <NA>          NA     NA    NA  2.22e- 1       NA          NA
      157          163 <NA>          NA     NA    NA  8.59e- 2       NA          NA
      158          164 <NA>          NA     NA    NA  1.80e- 2       NA          NA
      159          165 <NA>          NA     NA    NA  2.03e- 3       NA          NA
      160          166 <NA>          NA     NA    NA  1.23e- 4       NA          NA
      161          167 <NA>          NA     NA    NA  4.00e- 6       NA          NA
      162          168 <NA>          NA     NA    NA  6.96e- 8       NA          NA
      163          169 <NA>          NA     NA    NA  6.44e-10       NA          NA
      164          170 <NA>          NA     NA    NA  3.17e-12       NA          NA
      165          171 <NA>          NA     NA    NA  8.33e-15       NA          NA
      166          172 <NA>          NA     NA    NA  1.32e-20       NA          NA
      167          173 <NA>          NA     NA    NA  1.28e-17       NA          NA
      168          174 <NA>          NA     NA    NA  6.90e-15       NA          NA
      169          175 <NA>          NA     NA    NA  2.09e-12       NA          NA
      170          176 <NA>          NA     NA    NA  3.54e-10       NA          NA
      171          177 <NA>          NA     NA    NA  3.37e- 8       NA          NA
      172          178 <NA>          NA     NA    NA  1.81e- 6       NA          NA
      173          179 <NA>          NA     NA    NA  5.45e- 5       NA          NA
      174          180 <NA>          NA     NA    NA  9.31e- 4       NA          NA
      175          181 <NA>          NA     NA    NA  9.00e- 3       NA          NA
      176          182 <NA>          NA     NA    NA  4.95e- 2       NA          NA
      177          183 <NA>          NA     NA    NA  1.55e- 1       NA          NA
      178          184 <NA>          NA     NA    NA  2.76e- 1       NA          NA
      179          185 <NA>          NA     NA    NA  2.81e- 1       NA          NA
      180          186 <NA>          NA     NA    NA  1.63e- 1       NA          NA
      181          187 <NA>          NA     NA    NA  5.42e- 2       NA          NA
      182          188 <NA>          NA     NA    NA  1.02e- 2       NA          NA
      183          189 <NA>          NA     NA    NA  1.10e- 3       NA          NA
      184          190 <NA>          NA     NA    NA  6.66e- 5       NA          NA
      185          191 <NA>          NA     NA    NA  2.29e- 6       NA          NA
      186          192 <NA>          NA     NA    NA  4.44e- 8       NA          NA
      187          193 <NA>          NA     NA    NA  4.83e-10       NA          NA
      188          194 <NA>          NA     NA    NA  2.97e-12       NA          NA
      189          195 <NA>          NA     NA    NA  1.02e-20       NA          NA
      190          196 <NA>          NA     NA    NA  7.91e-18       NA          NA
      191          197 <NA>          NA     NA    NA  3.58e-15       NA          NA
      192          198 <NA>          NA     NA    NA  9.46e-13       NA          NA
      193          199 <NA>          NA     NA    NA  1.46e-10       NA          NA
      194          200 <NA>          NA     NA    NA  1.31e- 8       NA          NA
      195          201 <NA>          NA     NA    NA  6.94e- 7       NA          NA
      196          202 <NA>          NA     NA    NA  2.15e- 5       NA          NA
      197          203 <NA>          NA     NA    NA  3.91e- 4       NA          NA
      198          204 <NA>          NA     NA    NA  4.19e- 3       NA          NA
      199          205 <NA>          NA     NA    NA  2.65e- 2       NA          NA
      200          206 <NA>          NA     NA    NA  9.92e- 2       NA          NA
      201          207 <NA>          NA     NA    NA  2.20e- 1       NA          NA
      202          208 <NA>          NA     NA    NA  2.88e- 1       NA          NA
      203          209 <NA>          NA     NA    NA  2.25e- 1       NA          NA
      204          210 <NA>          NA     NA    NA  1.04e- 1       NA          NA
      205          211 <NA>          NA     NA    NA  2.83e- 2       NA          NA
      206          212 <NA>          NA     NA    NA  4.58e- 3       NA          NA
      207          213 <NA>          NA     NA    NA  4.37e- 4       NA          NA
      208          214 <NA>          NA     NA    NA  2.46e- 5       NA          NA
      209          215 <NA>          NA     NA    NA  8.11e- 7       NA          NA
      210          216 <NA>          NA     NA    NA  1.57e- 8       NA          NA
      211          217 <NA>          NA     NA    NA  1.80e-10       NA          NA
      212          218 <NA>          NA     NA    NA  8.31e-21       NA          NA
      213          219 <NA>          NA     NA    NA  5.43e-18       NA          NA
      214          220 <NA>          NA     NA    NA  2.14e-15       NA          NA
      215          221 <NA>          NA     NA    NA  5.06e-13       NA          NA
      216          222 <NA>          NA     NA    NA  7.20e-11       NA          NA
      217          223 <NA>          NA     NA    NA  6.18e- 9       NA          NA
      218          224 <NA>          NA     NA    NA  3.20e- 7       NA          NA
      219          225 <NA>          NA     NA    NA  1.00e- 5       NA          NA
      220          226 <NA>          NA     NA    NA  1.91e- 4       NA          NA
      221          227 <NA>          NA     NA    NA  2.19e- 3       NA          NA
      222          228 <NA>          NA     NA    NA  1.53e- 2       NA          NA
      223          229 <NA>          NA     NA    NA  6.53e- 2       NA          NA
      224          230 <NA>          NA     NA    NA  1.69e- 1       NA          NA
      225          231 <NA>          NA     NA    NA  2.68e- 1       NA          NA
      226          232 <NA>          NA     NA    NA  2.59e- 1       NA          NA
      227          233 <NA>          NA     NA    NA  1.53e- 1       NA          NA
      228          234 <NA>          NA     NA    NA  5.48e- 2       NA          NA
      229          235 <NA>          NA     NA    NA  1.20e- 2       NA          NA
      230          236 <NA>          NA     NA    NA  1.60e- 3       NA          NA
      231          237 <NA>          NA     NA    NA  1.30e- 4       NA          NA
      232          238 <NA>          NA     NA    NA  6.37e- 6       NA          NA
      233          239 <NA>          NA     NA    NA  1.89e- 7       NA          NA
      234          240 <NA>          NA     NA    NA  3.44e- 9       NA          NA
      235          241 <NA>          NA     NA    NA  7.07e-21       NA          NA
      236          242 <NA>          NA     NA    NA  4.03e-18       NA          NA
      237          243 <NA>          NA     NA    NA  1.42e-15       NA          NA
      238          244 <NA>          NA     NA    NA  3.06e-13       NA          NA
      239          245 <NA>          NA     NA    NA  4.09e-11       NA          NA
      240          246 <NA>          NA     NA    NA  3.36e- 9       NA          NA
      241          247 <NA>          NA     NA    NA  1.71e- 7       NA          NA
      242          248 <NA>          NA     NA    NA  5.39e- 6       NA          NA
      243          249 <NA>          NA     NA    NA  1.05e- 4       NA          NA
      244          250 <NA>          NA     NA    NA  1.27e- 3       NA          NA
      245          251 <NA>          NA     NA    NA  9.57e- 3       NA          NA
      246          252 <NA>          NA     NA    NA  4.48e- 2       NA          NA
      247          253 <NA>          NA     NA    NA  1.31e- 1       NA          NA
      248          254 <NA>          NA     NA    NA  2.38e- 1       NA          NA
      249          255 <NA>          NA     NA    NA  2.71e- 1       NA          NA
      250          256 <NA>          NA     NA    NA  1.92e- 1       NA          NA
      251          257 <NA>          NA     NA    NA  8.49e- 2       NA          NA
      252          258 <NA>          NA     NA    NA  2.34e- 2       NA          NA
      253          259 <NA>          NA     NA    NA  4.02e- 3       NA          NA
      254          260 <NA>          NA     NA    NA  4.30e- 4       NA          NA
      255          261 <NA>          NA     NA    NA  2.85e- 5       NA          NA
      256          262 <NA>          NA     NA    NA  1.17e- 6       NA          NA
      257          263 <NA>          NA     NA    NA  3.04e- 8       NA          NA
      258          264 <NA>          NA     NA    NA  6.22e-21       NA          NA
      259          265 <NA>          NA     NA    NA  3.17e-18       NA          NA
      260          266 <NA>          NA     NA    NA  1.02e-15       NA          NA
      261          267 <NA>          NA     NA    NA  2.04e-13       NA          NA
      262          268 <NA>          NA     NA    NA  2.58e-11       NA          NA
      263          269 <NA>          NA     NA    NA  2.05e- 9       NA          NA
      264          270 <NA>          NA     NA    NA  1.03e- 7       NA          NA
      265          271 <NA>          NA     NA    NA  3.23e- 6       NA          NA
      266          272 <NA>          NA     NA    NA  6.43e- 5       NA          NA
      267          273 <NA>          NA     NA    NA  8.07e- 4       NA          NA
      268          274 <NA>          NA     NA    NA  6.41e- 3       NA          NA
      269          275 <NA>          NA     NA    NA  3.23e- 2       NA          NA
      270          276 <NA>          NA     NA    NA  1.03e- 1       NA          NA
      271          277 <NA>          NA     NA    NA  2.09e- 1       NA          NA
      272          278 <NA>          NA     NA    NA  2.69e- 1       NA          NA
      273          279 <NA>          NA     NA    NA  2.20e- 1       NA          NA
      274          280 <NA>          NA     NA    NA  1.14e- 1       NA          NA
      275          281 <NA>          NA     NA    NA  3.76e- 2       NA          NA
      276          282 <NA>          NA     NA    NA  7.85e- 3       NA          NA
      277          283 <NA>          NA     NA    NA  1.04e- 3       NA          NA
      278          284 <NA>          NA     NA    NA  8.70e- 5       NA          NA
      279          285 <NA>          NA     NA    NA  4.61e- 6       NA          NA
      280          286 <NA>          NA     NA    NA  1.57e- 7       NA          NA
      281          287 <NA>          NA     NA    NA NA              NA          NA
      282          288 <NA>          NA     NA    NA NA              NA          NA
      283          289 <NA>          NA     NA    NA NA              NA          NA
      284          290 <NA>          NA     NA    NA NA              NA          NA
      285          291 <NA>          NA     NA    NA NA              NA          NA
      286          292 <NA>          NA     NA    NA NA              NA          NA
      287          293 <NA>          NA     NA    NA NA              NA          NA
      288          294 <NA>          NA     NA    NA NA              NA          NA
      289          295 <NA>          NA     NA    NA NA              NA          NA
      290          296 <NA>          NA     NA    NA NA              NA          NA
      291          297 <NA>          NA     NA    NA NA              NA          NA
      292          298 <NA>          NA     NA    NA NA              NA          NA
      293          299 <NA>          NA     NA    NA NA              NA          NA
      294          300 <NA>          NA     NA    NA NA              NA          NA
      295          301 <NA>          NA     NA    NA NA              NA          NA
      296          302 <NA>          NA     NA    NA NA              NA          NA
      297          303 <NA>          NA     NA    NA NA              NA          NA
      298          304 <NA>          NA     NA    NA NA              NA          NA
      299          305 <NA>          NA     NA    NA NA              NA          NA
      300          306 <NA>          NA     NA    NA NA              NA          NA
      301          307 <NA>          NA     NA    NA NA              NA          NA
      302          308 <NA>          NA     NA    NA NA              NA          NA
      303          309 <NA>          NA     NA    NA NA              NA          NA
      304          310 <NA>          NA     NA    NA NA              NA          NA
      305          311 <NA>          NA     NA    NA NA              NA          NA
      306          312 <NA>          NA     NA    NA NA              NA          NA
      307          313 <NA>          NA     NA    NA NA              NA          NA
      308          314 <NA>          NA     NA    NA NA              NA          NA
      309          315 <NA>          NA     NA    NA NA              NA          NA
      310          352 <NA>          NA     NA    NA NA              NA          NA
      311          353 <NA>          NA     NA    NA NA              NA          NA
      312          354 <NA>          NA     NA    NA NA              NA          NA
      313          355 <NA>          NA     NA    NA -2   e+ 2       NA          NA
      314          362 <NA>          NA     NA    NA  1.26e-16       NA          NA
      315          363 <NA>          NA     NA    NA  8.39e-11       NA          NA
      316          364 <NA>          NA     NA    NA  2.30e- 6       NA          NA
      317          365 <NA>          NA     NA    NA  2.74e- 3       NA          NA
      318          366 <NA>          NA     NA    NA  1.63e- 1       NA          NA
      319          367 <NA>          NA     NA    NA  6.32e- 1       NA          NA
      320          368 <NA>          NA     NA    NA  1.98e- 1       NA          NA
          estimation_type
          <chr>          
        1 fixed_effects  
        2 fixed_effects  
        3 constant       
        4 fixed_effects  
        5 constant       
        6 constant       
        7 constant       
        8 constant       
        9 constant       
       10 constant       
       11 constant       
       12 constant       
       13 constant       
       14 constant       
       15 constant       
       16 constant       
       17 constant       
       18 constant       
       19 constant       
       20 constant       
       21 constant       
       22 constant       
       23 constant       
       24 constant       
       25 constant       
       26 constant       
       27 constant       
       28 constant       
       29 constant       
       30 constant       
       31 constant       
       32 constant       
       33 constant       
       34 constant       
       35 constant       
       36 constant       
       37 constant       
       38 constant       
       39 constant       
       40 constant       
       41 constant       
       42 constant       
       43 constant       
       44 constant       
       45 constant       
       46 constant       
       47 constant       
       48 constant       
       49 constant       
       50 constant       
       51 constant       
       52 constant       
       53 constant       
       54 constant       
       55 constant       
       56 constant       
       57 constant       
       58 constant       
       59 constant       
       60 constant       
       61 constant       
       62 constant       
       63 constant       
       64 constant       
       65 constant       
       66 constant       
       67 constant       
       68 constant       
       69 constant       
       70 constant       
       71 constant       
       72 constant       
       73 constant       
       74 constant       
       75 constant       
       76 constant       
       77 constant       
       78 constant       
       79 constant       
       80 constant       
       81 constant       
       82 constant       
       83 constant       
       84 constant       
       85 constant       
       86 constant       
       87 constant       
       88 constant       
       89 constant       
       90 constant       
       91 constant       
       92 constant       
       93 constant       
       94 constant       
       95 constant       
       96 constant       
       97 constant       
       98 constant       
       99 constant       
      100 constant       
      101 constant       
      102 constant       
      103 constant       
      104 constant       
      105 constant       
      106 constant       
      107 constant       
      108 constant       
      109 constant       
      110 constant       
      111 constant       
      112 constant       
      113 constant       
      114 constant       
      115 constant       
      116 constant       
      117 constant       
      118 constant       
      119 constant       
      120 constant       
      121 constant       
      122 constant       
      123 constant       
      124 constant       
      125 constant       
      126 constant       
      127 constant       
      128 constant       
      129 constant       
      130 constant       
      131 constant       
      132 constant       
      133 constant       
      134 constant       
      135 constant       
      136 constant       
      137 constant       
      138 constant       
      139 constant       
      140 constant       
      141 constant       
      142 constant       
      143 constant       
      144 constant       
      145 constant       
      146 constant       
      147 constant       
      148 constant       
      149 constant       
      150 constant       
      151 constant       
      152 constant       
      153 constant       
      154 constant       
      155 constant       
      156 constant       
      157 constant       
      158 constant       
      159 constant       
      160 constant       
      161 constant       
      162 constant       
      163 constant       
      164 constant       
      165 constant       
      166 constant       
      167 constant       
      168 constant       
      169 constant       
      170 constant       
      171 constant       
      172 constant       
      173 constant       
      174 constant       
      175 constant       
      176 constant       
      177 constant       
      178 constant       
      179 constant       
      180 constant       
      181 constant       
      182 constant       
      183 constant       
      184 constant       
      185 constant       
      186 constant       
      187 constant       
      188 constant       
      189 constant       
      190 constant       
      191 constant       
      192 constant       
      193 constant       
      194 constant       
      195 constant       
      196 constant       
      197 constant       
      198 constant       
      199 constant       
      200 constant       
      201 constant       
      202 constant       
      203 constant       
      204 constant       
      205 constant       
      206 constant       
      207 constant       
      208 constant       
      209 constant       
      210 constant       
      211 constant       
      212 constant       
      213 constant       
      214 constant       
      215 constant       
      216 constant       
      217 constant       
      218 constant       
      219 constant       
      220 constant       
      221 constant       
      222 constant       
      223 constant       
      224 constant       
      225 constant       
      226 constant       
      227 constant       
      228 constant       
      229 constant       
      230 constant       
      231 constant       
      232 constant       
      233 constant       
      234 constant       
      235 constant       
      236 constant       
      237 constant       
      238 constant       
      239 constant       
      240 constant       
      241 constant       
      242 constant       
      243 constant       
      244 constant       
      245 constant       
      246 constant       
      247 constant       
      248 constant       
      249 constant       
      250 constant       
      251 constant       
      252 constant       
      253 constant       
      254 constant       
      255 constant       
      256 constant       
      257 constant       
      258 constant       
      259 constant       
      260 constant       
      261 constant       
      262 constant       
      263 constant       
      264 constant       
      265 constant       
      266 constant       
      267 constant       
      268 constant       
      269 constant       
      270 constant       
      271 constant       
      272 constant       
      273 constant       
      274 constant       
      275 constant       
      276 constant       
      277 constant       
      278 constant       
      279 constant       
      280 constant       
      281 fixed_effects  
      282 fixed_effects  
      283 fixed_effects  
      284 fixed_effects  
      285 fixed_effects  
      286 fixed_effects  
      287 fixed_effects  
      288 fixed_effects  
      289 fixed_effects  
      290 fixed_effects  
      291 fixed_effects  
      292 fixed_effects  
      293 fixed_effects  
      294 fixed_effects  
      295 fixed_effects  
      296 fixed_effects  
      297 fixed_effects  
      298 fixed_effects  
      299 fixed_effects  
      300 fixed_effects  
      301 fixed_effects  
      302 fixed_effects  
      303 fixed_effects  
      304 fixed_effects  
      305 fixed_effects  
      306 fixed_effects  
      307 fixed_effects  
      308 fixed_effects  
      309 fixed_effects  
      310 fixed_effects  
      311 fixed_effects  
      312 fixed_effects  
      313 constant       
      314 constant       
      315 constant       
      316 constant       
      317 constant       
      318 constant       
      319 constant       
      320 constant       
      # i 20,027 more rows

