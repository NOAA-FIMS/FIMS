# `get_estimates()` works with deterministic run

    Code
      estimates_snapshot <- dplyr::select(get_estimates(deterministic_results),
      -estimated, -expected, -uncertainty, -gradient, -likelihood, -log_like_cv)
      estimates_snapshot_lines <- capture.output(print(estimates_snapshot, n = 320,
        width = Inf))
      cat(trimws(estimates_snapshot_lines, which = "right"), sep = "\n")
    Output
      # A tibble: 13,996 x 18
          module_name module_id module_type label            type   type_id
          <chr>           <int> <chr>       <chr>            <chr>    <int>
        1 Fleet               1 <NA>        log_Fmort        vector       4
        2 Fleet               1 <NA>        log_Fmort        vector       4
        3 Fleet               1 <NA>        log_Fmort        vector       4
        4 Fleet               1 <NA>        log_Fmort        vector       4
        5 Fleet               1 <NA>        log_Fmort        vector       4
        6 Fleet               1 <NA>        log_Fmort        vector       4
        7 Fleet               1 <NA>        log_Fmort        vector       4
        8 Fleet               1 <NA>        log_Fmort        vector       4
        9 Fleet               1 <NA>        log_Fmort        vector       4
       10 Fleet               1 <NA>        log_Fmort        vector       4
       11 Fleet               1 <NA>        log_Fmort        vector       4
       12 Fleet               1 <NA>        log_Fmort        vector       4
       13 Fleet               1 <NA>        log_Fmort        vector       4
       14 Fleet               1 <NA>        log_Fmort        vector       4
       15 Fleet               1 <NA>        log_Fmort        vector       4
       16 Fleet               1 <NA>        log_Fmort        vector       4
       17 Fleet               1 <NA>        log_Fmort        vector       4
       18 Fleet               1 <NA>        log_Fmort        vector       4
       19 Fleet               1 <NA>        log_Fmort        vector       4
       20 Fleet               1 <NA>        log_Fmort        vector       4
       21 Fleet               1 <NA>        log_Fmort        vector       4
       22 Fleet               1 <NA>        log_Fmort        vector       4
       23 Fleet               1 <NA>        log_Fmort        vector       4
       24 Fleet               1 <NA>        log_Fmort        vector       4
       25 Fleet               1 <NA>        log_Fmort        vector       4
       26 Fleet               1 <NA>        log_Fmort        vector       4
       27 Fleet               1 <NA>        log_Fmort        vector       4
       28 Fleet               1 <NA>        log_Fmort        vector       4
       29 Fleet               1 <NA>        log_Fmort        vector       4
       30 Fleet               1 <NA>        log_Fmort        vector       4
       31 Fleet               1 <NA>        log_q            vector       3
       32 Fleet               1 <NA>        agecomp_expected <NA>        NA
       33 Fleet               1 <NA>        agecomp_expected <NA>        NA
       34 Fleet               1 <NA>        agecomp_expected <NA>        NA
       35 Fleet               1 <NA>        agecomp_expected <NA>        NA
       36 Fleet               1 <NA>        agecomp_expected <NA>        NA
       37 Fleet               1 <NA>        agecomp_expected <NA>        NA
       38 Fleet               1 <NA>        agecomp_expected <NA>        NA
       39 Fleet               1 <NA>        agecomp_expected <NA>        NA
       40 Fleet               1 <NA>        agecomp_expected <NA>        NA
       41 Fleet               1 <NA>        agecomp_expected <NA>        NA
       42 Fleet               1 <NA>        agecomp_expected <NA>        NA
       43 Fleet               1 <NA>        agecomp_expected <NA>        NA
       44 Fleet               1 <NA>        agecomp_expected <NA>        NA
       45 Fleet               1 <NA>        agecomp_expected <NA>        NA
       46 Fleet               1 <NA>        agecomp_expected <NA>        NA
       47 Fleet               1 <NA>        agecomp_expected <NA>        NA
       48 Fleet               1 <NA>        agecomp_expected <NA>        NA
       49 Fleet               1 <NA>        agecomp_expected <NA>        NA
       50 Fleet               1 <NA>        agecomp_expected <NA>        NA
       51 Fleet               1 <NA>        agecomp_expected <NA>        NA
       52 Fleet               1 <NA>        agecomp_expected <NA>        NA
       53 Fleet               1 <NA>        agecomp_expected <NA>        NA
       54 Fleet               1 <NA>        agecomp_expected <NA>        NA
       55 Fleet               1 <NA>        agecomp_expected <NA>        NA
       56 Fleet               1 <NA>        agecomp_expected <NA>        NA
       57 Fleet               1 <NA>        agecomp_expected <NA>        NA
       58 Fleet               1 <NA>        agecomp_expected <NA>        NA
       59 Fleet               1 <NA>        agecomp_expected <NA>        NA
       60 Fleet               1 <NA>        agecomp_expected <NA>        NA
       61 Fleet               1 <NA>        agecomp_expected <NA>        NA
       62 Fleet               1 <NA>        agecomp_expected <NA>        NA
       63 Fleet               1 <NA>        agecomp_expected <NA>        NA
       64 Fleet               1 <NA>        agecomp_expected <NA>        NA
       65 Fleet               1 <NA>        agecomp_expected <NA>        NA
       66 Fleet               1 <NA>        agecomp_expected <NA>        NA
       67 Fleet               1 <NA>        agecomp_expected <NA>        NA
       68 Fleet               1 <NA>        agecomp_expected <NA>        NA
       69 Fleet               1 <NA>        agecomp_expected <NA>        NA
       70 Fleet               1 <NA>        agecomp_expected <NA>        NA
       71 Fleet               1 <NA>        agecomp_expected <NA>        NA
       72 Fleet               1 <NA>        agecomp_expected <NA>        NA
       73 Fleet               1 <NA>        agecomp_expected <NA>        NA
       74 Fleet               1 <NA>        agecomp_expected <NA>        NA
       75 Fleet               1 <NA>        agecomp_expected <NA>        NA
       76 Fleet               1 <NA>        agecomp_expected <NA>        NA
       77 Fleet               1 <NA>        agecomp_expected <NA>        NA
       78 Fleet               1 <NA>        agecomp_expected <NA>        NA
       79 Fleet               1 <NA>        agecomp_expected <NA>        NA
       80 Fleet               1 <NA>        agecomp_expected <NA>        NA
       81 Fleet               1 <NA>        agecomp_expected <NA>        NA
       82 Fleet               1 <NA>        agecomp_expected <NA>        NA
       83 Fleet               1 <NA>        agecomp_expected <NA>        NA
       84 Fleet               1 <NA>        agecomp_expected <NA>        NA
       85 Fleet               1 <NA>        agecomp_expected <NA>        NA
       86 Fleet               1 <NA>        agecomp_expected <NA>        NA
       87 Fleet               1 <NA>        agecomp_expected <NA>        NA
       88 Fleet               1 <NA>        agecomp_expected <NA>        NA
       89 Fleet               1 <NA>        agecomp_expected <NA>        NA
       90 Fleet               1 <NA>        agecomp_expected <NA>        NA
       91 Fleet               1 <NA>        agecomp_expected <NA>        NA
       92 Fleet               1 <NA>        agecomp_expected <NA>        NA
       93 Fleet               1 <NA>        agecomp_expected <NA>        NA
       94 Fleet               1 <NA>        agecomp_expected <NA>        NA
       95 Fleet               1 <NA>        agecomp_expected <NA>        NA
       96 Fleet               1 <NA>        agecomp_expected <NA>        NA
       97 Fleet               1 <NA>        agecomp_expected <NA>        NA
       98 Fleet               1 <NA>        agecomp_expected <NA>        NA
       99 Fleet               1 <NA>        agecomp_expected <NA>        NA
      100 Fleet               1 <NA>        agecomp_expected <NA>        NA
      101 Fleet               1 <NA>        agecomp_expected <NA>        NA
      102 Fleet               1 <NA>        agecomp_expected <NA>        NA
      103 Fleet               1 <NA>        agecomp_expected <NA>        NA
      104 Fleet               1 <NA>        agecomp_expected <NA>        NA
      105 Fleet               1 <NA>        agecomp_expected <NA>        NA
      106 Fleet               1 <NA>        agecomp_expected <NA>        NA
      107 Fleet               1 <NA>        agecomp_expected <NA>        NA
      108 Fleet               1 <NA>        agecomp_expected <NA>        NA
      109 Fleet               1 <NA>        agecomp_expected <NA>        NA
      110 Fleet               1 <NA>        agecomp_expected <NA>        NA
      111 Fleet               1 <NA>        agecomp_expected <NA>        NA
      112 Fleet               1 <NA>        agecomp_expected <NA>        NA
      113 Fleet               1 <NA>        agecomp_expected <NA>        NA
      114 Fleet               1 <NA>        agecomp_expected <NA>        NA
      115 Fleet               1 <NA>        agecomp_expected <NA>        NA
      116 Fleet               1 <NA>        agecomp_expected <NA>        NA
      117 Fleet               1 <NA>        agecomp_expected <NA>        NA
      118 Fleet               1 <NA>        agecomp_expected <NA>        NA
      119 Fleet               1 <NA>        agecomp_expected <NA>        NA
      120 Fleet               1 <NA>        agecomp_expected <NA>        NA
      121 Fleet               1 <NA>        agecomp_expected <NA>        NA
      122 Fleet               1 <NA>        agecomp_expected <NA>        NA
      123 Fleet               1 <NA>        agecomp_expected <NA>        NA
      124 Fleet               1 <NA>        agecomp_expected <NA>        NA
      125 Fleet               1 <NA>        agecomp_expected <NA>        NA
      126 Fleet               1 <NA>        agecomp_expected <NA>        NA
      127 Fleet               1 <NA>        agecomp_expected <NA>        NA
      128 Fleet               1 <NA>        agecomp_expected <NA>        NA
      129 Fleet               1 <NA>        agecomp_expected <NA>        NA
      130 Fleet               1 <NA>        agecomp_expected <NA>        NA
      131 Fleet               1 <NA>        agecomp_expected <NA>        NA
      132 Fleet               1 <NA>        agecomp_expected <NA>        NA
      133 Fleet               1 <NA>        agecomp_expected <NA>        NA
      134 Fleet               1 <NA>        agecomp_expected <NA>        NA
      135 Fleet               1 <NA>        agecomp_expected <NA>        NA
      136 Fleet               1 <NA>        agecomp_expected <NA>        NA
      137 Fleet               1 <NA>        agecomp_expected <NA>        NA
      138 Fleet               1 <NA>        agecomp_expected <NA>        NA
      139 Fleet               1 <NA>        agecomp_expected <NA>        NA
      140 Fleet               1 <NA>        agecomp_expected <NA>        NA
      141 Fleet               1 <NA>        agecomp_expected <NA>        NA
      142 Fleet               1 <NA>        agecomp_expected <NA>        NA
      143 Fleet               1 <NA>        agecomp_expected <NA>        NA
      144 Fleet               1 <NA>        agecomp_expected <NA>        NA
      145 Fleet               1 <NA>        agecomp_expected <NA>        NA
      146 Fleet               1 <NA>        agecomp_expected <NA>        NA
      147 Fleet               1 <NA>        agecomp_expected <NA>        NA
      148 Fleet               1 <NA>        agecomp_expected <NA>        NA
      149 Fleet               1 <NA>        agecomp_expected <NA>        NA
      150 Fleet               1 <NA>        agecomp_expected <NA>        NA
      151 Fleet               1 <NA>        agecomp_expected <NA>        NA
      152 Fleet               1 <NA>        agecomp_expected <NA>        NA
      153 Fleet               1 <NA>        agecomp_expected <NA>        NA
      154 Fleet               1 <NA>        agecomp_expected <NA>        NA
      155 Fleet               1 <NA>        agecomp_expected <NA>        NA
      156 Fleet               1 <NA>        agecomp_expected <NA>        NA
      157 Fleet               1 <NA>        agecomp_expected <NA>        NA
      158 Fleet               1 <NA>        agecomp_expected <NA>        NA
      159 Fleet               1 <NA>        agecomp_expected <NA>        NA
      160 Fleet               1 <NA>        agecomp_expected <NA>        NA
      161 Fleet               1 <NA>        agecomp_expected <NA>        NA
      162 Fleet               1 <NA>        agecomp_expected <NA>        NA
      163 Fleet               1 <NA>        agecomp_expected <NA>        NA
      164 Fleet               1 <NA>        agecomp_expected <NA>        NA
      165 Fleet               1 <NA>        agecomp_expected <NA>        NA
      166 Fleet               1 <NA>        agecomp_expected <NA>        NA
      167 Fleet               1 <NA>        agecomp_expected <NA>        NA
      168 Fleet               1 <NA>        agecomp_expected <NA>        NA
      169 Fleet               1 <NA>        agecomp_expected <NA>        NA
      170 Fleet               1 <NA>        agecomp_expected <NA>        NA
      171 Fleet               1 <NA>        agecomp_expected <NA>        NA
      172 Fleet               1 <NA>        agecomp_expected <NA>        NA
      173 Fleet               1 <NA>        agecomp_expected <NA>        NA
      174 Fleet               1 <NA>        agecomp_expected <NA>        NA
      175 Fleet               1 <NA>        agecomp_expected <NA>        NA
      176 Fleet               1 <NA>        agecomp_expected <NA>        NA
      177 Fleet               1 <NA>        agecomp_expected <NA>        NA
      178 Fleet               1 <NA>        agecomp_expected <NA>        NA
      179 Fleet               1 <NA>        agecomp_expected <NA>        NA
      180 Fleet               1 <NA>        agecomp_expected <NA>        NA
      181 Fleet               1 <NA>        agecomp_expected <NA>        NA
      182 Fleet               1 <NA>        agecomp_expected <NA>        NA
      183 Fleet               1 <NA>        agecomp_expected <NA>        NA
      184 Fleet               1 <NA>        agecomp_expected <NA>        NA
      185 Fleet               1 <NA>        agecomp_expected <NA>        NA
      186 Fleet               1 <NA>        agecomp_expected <NA>        NA
      187 Fleet               1 <NA>        agecomp_expected <NA>        NA
      188 Fleet               1 <NA>        agecomp_expected <NA>        NA
      189 Fleet               1 <NA>        agecomp_expected <NA>        NA
      190 Fleet               1 <NA>        agecomp_expected <NA>        NA
      191 Fleet               1 <NA>        agecomp_expected <NA>        NA
      192 Fleet               1 <NA>        agecomp_expected <NA>        NA
      193 Fleet               1 <NA>        agecomp_expected <NA>        NA
      194 Fleet               1 <NA>        agecomp_expected <NA>        NA
      195 Fleet               1 <NA>        agecomp_expected <NA>        NA
      196 Fleet               1 <NA>        agecomp_expected <NA>        NA
      197 Fleet               1 <NA>        agecomp_expected <NA>        NA
      198 Fleet               1 <NA>        agecomp_expected <NA>        NA
      199 Fleet               1 <NA>        agecomp_expected <NA>        NA
      200 Fleet               1 <NA>        agecomp_expected <NA>        NA
      201 Fleet               1 <NA>        agecomp_expected <NA>        NA
      202 Fleet               1 <NA>        agecomp_expected <NA>        NA
      203 Fleet               1 <NA>        agecomp_expected <NA>        NA
      204 Fleet               1 <NA>        agecomp_expected <NA>        NA
      205 Fleet               1 <NA>        agecomp_expected <NA>        NA
      206 Fleet               1 <NA>        agecomp_expected <NA>        NA
      207 Fleet               1 <NA>        agecomp_expected <NA>        NA
      208 Fleet               1 <NA>        agecomp_expected <NA>        NA
      209 Fleet               1 <NA>        agecomp_expected <NA>        NA
      210 Fleet               1 <NA>        agecomp_expected <NA>        NA
      211 Fleet               1 <NA>        agecomp_expected <NA>        NA
      212 Fleet               1 <NA>        agecomp_expected <NA>        NA
      213 Fleet               1 <NA>        agecomp_expected <NA>        NA
      214 Fleet               1 <NA>        agecomp_expected <NA>        NA
      215 Fleet               1 <NA>        agecomp_expected <NA>        NA
      216 Fleet               1 <NA>        agecomp_expected <NA>        NA
      217 Fleet               1 <NA>        agecomp_expected <NA>        NA
      218 Fleet               1 <NA>        agecomp_expected <NA>        NA
      219 Fleet               1 <NA>        agecomp_expected <NA>        NA
      220 Fleet               1 <NA>        agecomp_expected <NA>        NA
      221 Fleet               1 <NA>        agecomp_expected <NA>        NA
      222 Fleet               1 <NA>        agecomp_expected <NA>        NA
      223 Fleet               1 <NA>        agecomp_expected <NA>        NA
      224 Fleet               1 <NA>        agecomp_expected <NA>        NA
      225 Fleet               1 <NA>        agecomp_expected <NA>        NA
      226 Fleet               1 <NA>        agecomp_expected <NA>        NA
      227 Fleet               1 <NA>        agecomp_expected <NA>        NA
      228 Fleet               1 <NA>        agecomp_expected <NA>        NA
      229 Fleet               1 <NA>        agecomp_expected <NA>        NA
      230 Fleet               1 <NA>        agecomp_expected <NA>        NA
      231 Fleet               1 <NA>        agecomp_expected <NA>        NA
      232 Fleet               1 <NA>        agecomp_expected <NA>        NA
      233 Fleet               1 <NA>        agecomp_expected <NA>        NA
      234 Fleet               1 <NA>        agecomp_expected <NA>        NA
      235 Fleet               1 <NA>        agecomp_expected <NA>        NA
      236 Fleet               1 <NA>        agecomp_expected <NA>        NA
      237 Fleet               1 <NA>        agecomp_expected <NA>        NA
      238 Fleet               1 <NA>        agecomp_expected <NA>        NA
      239 Fleet               1 <NA>        agecomp_expected <NA>        NA
      240 Fleet               1 <NA>        agecomp_expected <NA>        NA
      241 Fleet               1 <NA>        agecomp_expected <NA>        NA
      242 Fleet               1 <NA>        agecomp_expected <NA>        NA
      243 Fleet               1 <NA>        agecomp_expected <NA>        NA
      244 Fleet               1 <NA>        agecomp_expected <NA>        NA
      245 Fleet               1 <NA>        agecomp_expected <NA>        NA
      246 Fleet               1 <NA>        agecomp_expected <NA>        NA
      247 Fleet               1 <NA>        agecomp_expected <NA>        NA
      248 Fleet               1 <NA>        agecomp_expected <NA>        NA
      249 Fleet               1 <NA>        agecomp_expected <NA>        NA
      250 Fleet               1 <NA>        agecomp_expected <NA>        NA
      251 Fleet               1 <NA>        agecomp_expected <NA>        NA
      252 Fleet               1 <NA>        agecomp_expected <NA>        NA
      253 Fleet               1 <NA>        agecomp_expected <NA>        NA
      254 Fleet               1 <NA>        agecomp_expected <NA>        NA
      255 Fleet               1 <NA>        agecomp_expected <NA>        NA
      256 Fleet               1 <NA>        agecomp_expected <NA>        NA
      257 Fleet               1 <NA>        agecomp_expected <NA>        NA
      258 Fleet               1 <NA>        agecomp_expected <NA>        NA
      259 Fleet               1 <NA>        agecomp_expected <NA>        NA
      260 Fleet               1 <NA>        agecomp_expected <NA>        NA
      261 Fleet               1 <NA>        agecomp_expected <NA>        NA
      262 Fleet               1 <NA>        agecomp_expected <NA>        NA
      263 Fleet               1 <NA>        agecomp_expected <NA>        NA
      264 Fleet               1 <NA>        agecomp_expected <NA>        NA
      265 Fleet               1 <NA>        agecomp_expected <NA>        NA
      266 Fleet               1 <NA>        agecomp_expected <NA>        NA
      267 Fleet               1 <NA>        agecomp_expected <NA>        NA
      268 Fleet               1 <NA>        agecomp_expected <NA>        NA
      269 Fleet               1 <NA>        agecomp_expected <NA>        NA
      270 Fleet               1 <NA>        agecomp_expected <NA>        NA
      271 Fleet               1 <NA>        agecomp_expected <NA>        NA
      272 Fleet               1 <NA>        agecomp_expected <NA>        NA
      273 Fleet               1 <NA>        agecomp_expected <NA>        NA
      274 Fleet               1 <NA>        agecomp_expected <NA>        NA
      275 Fleet               1 <NA>        agecomp_expected <NA>        NA
      276 Fleet               1 <NA>        agecomp_expected <NA>        NA
      277 Fleet               1 <NA>        agecomp_expected <NA>        NA
      278 Fleet               1 <NA>        agecomp_expected <NA>        NA
      279 Fleet               1 <NA>        agecomp_expected <NA>        NA
      280 Fleet               1 <NA>        agecomp_expected <NA>        NA
      281 Fleet               1 <NA>        agecomp_expected <NA>        NA
      282 Fleet               1 <NA>        agecomp_expected <NA>        NA
      283 Fleet               1 <NA>        agecomp_expected <NA>        NA
      284 Fleet               1 <NA>        agecomp_expected <NA>        NA
      285 Fleet               1 <NA>        agecomp_expected <NA>        NA
      286 Fleet               1 <NA>        agecomp_expected <NA>        NA
      287 Fleet               1 <NA>        agecomp_expected <NA>        NA
      288 Fleet               1 <NA>        agecomp_expected <NA>        NA
      289 Fleet               1 <NA>        agecomp_expected <NA>        NA
      290 Fleet               1 <NA>        agecomp_expected <NA>        NA
      291 Fleet               1 <NA>        agecomp_expected <NA>        NA
      292 Fleet               1 <NA>        agecomp_expected <NA>        NA
      293 Fleet               1 <NA>        agecomp_expected <NA>        NA
      294 Fleet               1 <NA>        agecomp_expected <NA>        NA
      295 Fleet               1 <NA>        agecomp_expected <NA>        NA
      296 Fleet               1 <NA>        agecomp_expected <NA>        NA
      297 Fleet               1 <NA>        agecomp_expected <NA>        NA
      298 Fleet               1 <NA>        agecomp_expected <NA>        NA
      299 Fleet               1 <NA>        agecomp_expected <NA>        NA
      300 Fleet               1 <NA>        agecomp_expected <NA>        NA
      301 Fleet               1 <NA>        agecomp_expected <NA>        NA
      302 Fleet               1 <NA>        agecomp_expected <NA>        NA
      303 Fleet               1 <NA>        agecomp_expected <NA>        NA
      304 Fleet               1 <NA>        agecomp_expected <NA>        NA
      305 Fleet               1 <NA>        agecomp_expected <NA>        NA
      306 Fleet               1 <NA>        agecomp_expected <NA>        NA
      307 Fleet               1 <NA>        agecomp_expected <NA>        NA
      308 Fleet               1 <NA>        agecomp_expected <NA>        NA
      309 Fleet               1 <NA>        agecomp_expected <NA>        NA
      310 Fleet               1 <NA>        agecomp_expected <NA>        NA
      311 Fleet               1 <NA>        agecomp_expected <NA>        NA
      312 Fleet               1 <NA>        agecomp_expected <NA>        NA
      313 Fleet               1 <NA>        agecomp_expected <NA>        NA
      314 Fleet               1 <NA>        agecomp_expected <NA>        NA
      315 Fleet               1 <NA>        agecomp_expected <NA>        NA
      316 Fleet               1 <NA>        agecomp_expected <NA>        NA
      317 Fleet               1 <NA>        agecomp_expected <NA>        NA
      318 Fleet               1 <NA>        agecomp_expected <NA>        NA
      319 Fleet               1 <NA>        agecomp_expected <NA>        NA
      320 Fleet               1 <NA>        agecomp_expected <NA>        NA
          parameter_id fleet  year_i age_i length_i  input observed estimation_type
                 <int> <chr>   <int> <int>    <int>  <dbl>    <dbl> <chr>
        1            4 fleet1      1    NA       NA -4.66        NA fixed_effects
        2           24 fleet1      2    NA       NA -3.60        NA fixed_effects
        3           25 fleet1      3    NA       NA -3.10        NA fixed_effects
        4           26 fleet1      4    NA       NA -2.80        NA fixed_effects
        5           27 fleet1      5    NA       NA -3.02        NA fixed_effects
        6           28 fleet1      6    NA       NA -2.44        NA fixed_effects
        7           29 fleet1      7    NA       NA -2.43        NA fixed_effects
        8           30 fleet1      8    NA       NA -1.68        NA fixed_effects
        9           31 fleet1      9    NA       NA -2.22        NA fixed_effects
       10           32 fleet1     10    NA       NA -2.02        NA fixed_effects
       11           33 fleet1     11    NA       NA -1.89        NA fixed_effects
       12           34 fleet1     12    NA       NA -1.82        NA fixed_effects
       13           35 fleet1     13    NA       NA -2.15        NA fixed_effects
       14           36 fleet1     14    NA       NA -1.78        NA fixed_effects
       15           37 fleet1     15    NA       NA -1.71        NA fixed_effects
       16           38 fleet1     16    NA       NA -1.82        NA fixed_effects
       17           39 fleet1     17    NA       NA -1.16        NA fixed_effects
       18           40 fleet1     18    NA       NA -1.36        NA fixed_effects
       19           41 fleet1     19    NA       NA -1.37        NA fixed_effects
       20           42 fleet1     20    NA       NA -1.38        NA fixed_effects
       21           43 fleet1     21    NA       NA -1.05        NA fixed_effects
       22           44 fleet1     22    NA       NA -1.37        NA fixed_effects
       23           45 fleet1     23    NA       NA -0.871       NA fixed_effects
       24           46 fleet1     24    NA       NA -1.06        NA fixed_effects
       25           47 fleet1     25    NA       NA -1.07        NA fixed_effects
       26           48 fleet1     26    NA       NA -1.16        NA fixed_effects
       27           49 fleet1     27    NA       NA -1.18        NA fixed_effects
       28           50 fleet1     28    NA       NA -0.840       NA fixed_effects
       29           51 fleet1     29    NA       NA -1.11        NA fixed_effects
       30           52 fleet1     30    NA       NA -0.694       NA fixed_effects
       31            3 fleet1     NA    NA       NA  0           NA constant
       32           NA fleet1      1     1       NA NA           14 derived_quantity
       33           NA fleet1      1     2       NA NA           20 derived_quantity
       34           NA fleet1      1     3       NA NA           23 derived_quantity
       35           NA fleet1      1     4       NA NA           30 derived_quantity
       36           NA fleet1      1     5       NA NA           20 derived_quantity
       37           NA fleet1      1     6       NA NA           10 derived_quantity
       38           NA fleet1      1     7       NA NA           22 derived_quantity
       39           NA fleet1      1     8       NA NA           11 derived_quantity
       40           NA fleet1      1     9       NA NA           12 derived_quantity
       41           NA fleet1      1    10       NA NA            6 derived_quantity
       42           NA fleet1      1    11       NA NA            7 derived_quantity
       43           NA fleet1      1    12       NA NA           25 derived_quantity
       44           NA fleet1      2     1       NA NA           21 derived_quantity
       45           NA fleet1      2     2       NA NA           19 derived_quantity
       46           NA fleet1      2     3       NA NA           22 derived_quantity
       47           NA fleet1      2     4       NA NA           18 derived_quantity
       48           NA fleet1      2     5       NA NA           31 derived_quantity
       49           NA fleet1      2     6       NA NA           11 derived_quantity
       50           NA fleet1      2     7       NA NA           13 derived_quantity
       51           NA fleet1      2     8       NA NA           17 derived_quantity
       52           NA fleet1      2     9       NA NA            9 derived_quantity
       53           NA fleet1      2    10       NA NA            7 derived_quantity
       54           NA fleet1      2    11       NA NA            5 derived_quantity
       55           NA fleet1      2    12       NA NA           27 derived_quantity
       56           NA fleet1      3     1       NA NA           11 derived_quantity
       57           NA fleet1      3     2       NA NA           30 derived_quantity
       58           NA fleet1      3     3       NA NA           21 derived_quantity
       59           NA fleet1      3     4       NA NA           23 derived_quantity
       60           NA fleet1      3     5       NA NA           19 derived_quantity
       61           NA fleet1      3     6       NA NA           18 derived_quantity
       62           NA fleet1      3     7       NA NA           14 derived_quantity
       63           NA fleet1      3     8       NA NA           17 derived_quantity
       64           NA fleet1      3     9       NA NA            6 derived_quantity
       65           NA fleet1      3    10       NA NA            9 derived_quantity
       66           NA fleet1      3    11       NA NA            6 derived_quantity
       67           NA fleet1      3    12       NA NA           26 derived_quantity
       68           NA fleet1      4     1       NA NA           10 derived_quantity
       69           NA fleet1      4     2       NA NA           11 derived_quantity
       70           NA fleet1      4     3       NA NA           44 derived_quantity
       71           NA fleet1      4     4       NA NA           20 derived_quantity
       72           NA fleet1      4     5       NA NA           20 derived_quantity
       73           NA fleet1      4     6       NA NA           21 derived_quantity
       74           NA fleet1      4     7       NA NA           13 derived_quantity
       75           NA fleet1      4     8       NA NA           11 derived_quantity
       76           NA fleet1      4     9       NA NA           13 derived_quantity
       77           NA fleet1      4    10       NA NA            6 derived_quantity
       78           NA fleet1      4    11       NA NA            3 derived_quantity
       79           NA fleet1      4    12       NA NA           28 derived_quantity
       80           NA fleet1      5     1       NA NA           31 derived_quantity
       81           NA fleet1      5     2       NA NA           10 derived_quantity
       82           NA fleet1      5     3       NA NA           32 derived_quantity
       83           NA fleet1      5     4       NA NA           34 derived_quantity
       84           NA fleet1      5     5       NA NA           18 derived_quantity
       85           NA fleet1      5     6       NA NA           19 derived_quantity
       86           NA fleet1      5     7       NA NA            9 derived_quantity
       87           NA fleet1      5     8       NA NA            9 derived_quantity
       88           NA fleet1      5     9       NA NA            6 derived_quantity
       89           NA fleet1      5    10       NA NA            4 derived_quantity
       90           NA fleet1      5    11       NA NA            6 derived_quantity
       91           NA fleet1      5    12       NA NA           22 derived_quantity
       92           NA fleet1      6     1       NA NA           16 derived_quantity
       93           NA fleet1      6     2       NA NA           37 derived_quantity
       94           NA fleet1      6     3       NA NA           14 derived_quantity
       95           NA fleet1      6     4       NA NA           18 derived_quantity
       96           NA fleet1      6     5       NA NA           29 derived_quantity
       97           NA fleet1      6     6       NA NA           15 derived_quantity
       98           NA fleet1      6     7       NA NA           12 derived_quantity
       99           NA fleet1      6     8       NA NA           13 derived_quantity
      100           NA fleet1      6     9       NA NA            8 derived_quantity
      101           NA fleet1      6    10       NA NA            9 derived_quantity
      102           NA fleet1      6    11       NA NA            5 derived_quantity
      103           NA fleet1      6    12       NA NA           24 derived_quantity
      104           NA fleet1      7     1       NA NA           11 derived_quantity
      105           NA fleet1      7     2       NA NA           26 derived_quantity
      106           NA fleet1      7     3       NA NA           44 derived_quantity
      107           NA fleet1      7     4       NA NA           17 derived_quantity
      108           NA fleet1      7     5       NA NA           19 derived_quantity
      109           NA fleet1      7     6       NA NA           28 derived_quantity
      110           NA fleet1      7     7       NA NA            7 derived_quantity
      111           NA fleet1      7     8       NA NA            8 derived_quantity
      112           NA fleet1      7     9       NA NA            7 derived_quantity
      113           NA fleet1      7    10       NA NA            4 derived_quantity
      114           NA fleet1      7    11       NA NA            5 derived_quantity
      115           NA fleet1      7    12       NA NA           24 derived_quantity
      116           NA fleet1      8     1       NA NA           18 derived_quantity
      117           NA fleet1      8     2       NA NA           15 derived_quantity
      118           NA fleet1      8     3       NA NA           44 derived_quantity
      119           NA fleet1      8     4       NA NA           40 derived_quantity
      120           NA fleet1      8     5       NA NA           12 derived_quantity
      121           NA fleet1      8     6       NA NA            8 derived_quantity
      122           NA fleet1      8     7       NA NA           14 derived_quantity
      123           NA fleet1      8     8       NA NA           13 derived_quantity
      124           NA fleet1      8     9       NA NA            5 derived_quantity
      125           NA fleet1      8    10       NA NA            5 derived_quantity
      126           NA fleet1      8    11       NA NA            5 derived_quantity
      127           NA fleet1      8    12       NA NA           21 derived_quantity
      128           NA fleet1      9     1       NA NA           14 derived_quantity
      129           NA fleet1      9     2       NA NA           32 derived_quantity
      130           NA fleet1      9     3       NA NA           27 derived_quantity
      131           NA fleet1      9     4       NA NA           35 derived_quantity
      132           NA fleet1      9     5       NA NA           31 derived_quantity
      133           NA fleet1      9     6       NA NA            9 derived_quantity
      134           NA fleet1      9     7       NA NA            9 derived_quantity
      135           NA fleet1      9     8       NA NA           12 derived_quantity
      136           NA fleet1      9     9       NA NA            5 derived_quantity
      137           NA fleet1      9    10       NA NA            5 derived_quantity
      138           NA fleet1      9    11       NA NA            3 derived_quantity
      139           NA fleet1      9    12       NA NA           18 derived_quantity
      140           NA fleet1     10     1       NA NA           15 derived_quantity
      141           NA fleet1     10     2       NA NA           21 derived_quantity
      142           NA fleet1     10     3       NA NA           43 derived_quantity
      143           NA fleet1     10     4       NA NA           20 derived_quantity
      144           NA fleet1     10     5       NA NA           22 derived_quantity
      145           NA fleet1     10     6       NA NA           21 derived_quantity
      146           NA fleet1     10     7       NA NA            7 derived_quantity
      147           NA fleet1     10     8       NA NA            8 derived_quantity
      148           NA fleet1     10     9       NA NA           13 derived_quantity
      149           NA fleet1     10    10       NA NA            4 derived_quantity
      150           NA fleet1     10    11       NA NA            4 derived_quantity
      151           NA fleet1     10    12       NA NA           22 derived_quantity
      152           NA fleet1     11     1       NA NA           15 derived_quantity
      153           NA fleet1     11     2       NA NA           34 derived_quantity
      154           NA fleet1     11     3       NA NA           23 derived_quantity
      155           NA fleet1     11     4       NA NA           43 derived_quantity
      156           NA fleet1     11     5       NA NA           15 derived_quantity
      157           NA fleet1     11     6       NA NA           23 derived_quantity
      158           NA fleet1     11     7       NA NA           21 derived_quantity
      159           NA fleet1     11     8       NA NA            5 derived_quantity
      160           NA fleet1     11     9       NA NA            3 derived_quantity
      161           NA fleet1     11    10       NA NA            5 derived_quantity
      162           NA fleet1     11    11       NA NA            2 derived_quantity
      163           NA fleet1     11    12       NA NA           11 derived_quantity
      164           NA fleet1     12     1       NA NA           11 derived_quantity
      165           NA fleet1     12     2       NA NA           32 derived_quantity
      166           NA fleet1     12     3       NA NA           30 derived_quantity
      167           NA fleet1     12     4       NA NA           26 derived_quantity
      168           NA fleet1     12     5       NA NA           33 derived_quantity
      169           NA fleet1     12     6       NA NA           14 derived_quantity
      170           NA fleet1     12     7       NA NA           13 derived_quantity
      171           NA fleet1     12     8       NA NA           21 derived_quantity
      172           NA fleet1     12     9       NA NA            4 derived_quantity
      173           NA fleet1     12    10       NA NA            3 derived_quantity
      174           NA fleet1     12    11       NA NA            4 derived_quantity
      175           NA fleet1     12    12       NA NA            9 derived_quantity
      176           NA fleet1     13     1       NA NA           22 derived_quantity
      177           NA fleet1     13     2       NA NA           21 derived_quantity
      178           NA fleet1     13     3       NA NA           23 derived_quantity
      179           NA fleet1     13     4       NA NA           28 derived_quantity
      180           NA fleet1     13     5       NA NA           23 derived_quantity
      181           NA fleet1     13     6       NA NA           18 derived_quantity
      182           NA fleet1     13     7       NA NA           15 derived_quantity
      183           NA fleet1     13     8       NA NA           18 derived_quantity
      184           NA fleet1     13     9       NA NA           16 derived_quantity
      185           NA fleet1     13    10       NA NA            3 derived_quantity
      186           NA fleet1     13    11       NA NA            3 derived_quantity
      187           NA fleet1     13    12       NA NA           10 derived_quantity
      188           NA fleet1     14     1       NA NA           23 derived_quantity
      189           NA fleet1     14     2       NA NA           28 derived_quantity
      190           NA fleet1     14     3       NA NA           17 derived_quantity
      191           NA fleet1     14     4       NA NA           41 derived_quantity
      192           NA fleet1     14     5       NA NA           24 derived_quantity
      193           NA fleet1     14     6       NA NA           10 derived_quantity
      194           NA fleet1     14     7       NA NA           10 derived_quantity
      195           NA fleet1     14     8       NA NA            8 derived_quantity
      196           NA fleet1     14     9       NA NA            9 derived_quantity
      197           NA fleet1     14    10       NA NA           12 derived_quantity
      198           NA fleet1     14    11       NA NA            5 derived_quantity
      199           NA fleet1     14    12       NA NA           13 derived_quantity
      200           NA fleet1     15     1       NA NA           14 derived_quantity
      201           NA fleet1     15     2       NA NA           38 derived_quantity
      202           NA fleet1     15     3       NA NA           39 derived_quantity
      203           NA fleet1     15     4       NA NA           16 derived_quantity
      204           NA fleet1     15     5       NA NA           28 derived_quantity
      205           NA fleet1     15     6       NA NA           21 derived_quantity
      206           NA fleet1     15     7       NA NA           10 derived_quantity
      207           NA fleet1     15     8       NA NA           11 derived_quantity
      208           NA fleet1     15     9       NA NA            5 derived_quantity
      209           NA fleet1     15    10       NA NA            4 derived_quantity
      210           NA fleet1     15    11       NA NA            0 derived_quantity
      211           NA fleet1     15    12       NA NA           14 derived_quantity
      212           NA fleet1     16     1       NA NA           29 derived_quantity
      213           NA fleet1     16     2       NA NA           25 derived_quantity
      214           NA fleet1     16     3       NA NA           39 derived_quantity
      215           NA fleet1     16     4       NA NA           25 derived_quantity
      216           NA fleet1     16     5       NA NA           17 derived_quantity
      217           NA fleet1     16     6       NA NA           13 derived_quantity
      218           NA fleet1     16     7       NA NA           12 derived_quantity
      219           NA fleet1     16     8       NA NA            8 derived_quantity
      220           NA fleet1     16     9       NA NA            7 derived_quantity
      221           NA fleet1     16    10       NA NA            3 derived_quantity
      222           NA fleet1     16    11       NA NA            7 derived_quantity
      223           NA fleet1     16    12       NA NA           15 derived_quantity
      224           NA fleet1     17     1       NA NA           33 derived_quantity
      225           NA fleet1     17     2       NA NA           38 derived_quantity
      226           NA fleet1     17     3       NA NA           28 derived_quantity
      227           NA fleet1     17     4       NA NA           29 derived_quantity
      228           NA fleet1     17     5       NA NA           24 derived_quantity
      229           NA fleet1     17     6       NA NA            6 derived_quantity
      230           NA fleet1     17     7       NA NA           10 derived_quantity
      231           NA fleet1     17     8       NA NA            1 derived_quantity
      232           NA fleet1     17     9       NA NA            6 derived_quantity
      233           NA fleet1     17    10       NA NA            5 derived_quantity
      234           NA fleet1     17    11       NA NA            2 derived_quantity
      235           NA fleet1     17    12       NA NA           18 derived_quantity
      236           NA fleet1     18     1       NA NA           17 derived_quantity
      237           NA fleet1     18     2       NA NA           40 derived_quantity
      238           NA fleet1     18     3       NA NA           50 derived_quantity
      239           NA fleet1     18     4       NA NA           25 derived_quantity
      240           NA fleet1     18     5       NA NA           17 derived_quantity
      241           NA fleet1     18     6       NA NA           13 derived_quantity
      242           NA fleet1     18     7       NA NA           11 derived_quantity
      243           NA fleet1     18     8       NA NA            8 derived_quantity
      244           NA fleet1     18     9       NA NA            7 derived_quantity
      245           NA fleet1     18    10       NA NA            1 derived_quantity
      246           NA fleet1     18    11       NA NA            4 derived_quantity
      247           NA fleet1     18    12       NA NA            7 derived_quantity
      248           NA fleet1     19     1       NA NA           26 derived_quantity
      249           NA fleet1     19     2       NA NA           24 derived_quantity
      250           NA fleet1     19     3       NA NA           37 derived_quantity
      251           NA fleet1     19     4       NA NA           34 derived_quantity
      252           NA fleet1     19     5       NA NA           22 derived_quantity
      253           NA fleet1     19     6       NA NA           23 derived_quantity
      254           NA fleet1     19     7       NA NA            8 derived_quantity
      255           NA fleet1     19     8       NA NA            6 derived_quantity
      256           NA fleet1     19     9       NA NA            3 derived_quantity
      257           NA fleet1     19    10       NA NA            5 derived_quantity
      258           NA fleet1     19    11       NA NA            4 derived_quantity
      259           NA fleet1     19    12       NA NA            8 derived_quantity
      260           NA fleet1     20     1       NA NA            9 derived_quantity
      261           NA fleet1     20     2       NA NA           31 derived_quantity
      262           NA fleet1     20     3       NA NA           33 derived_quantity
      263           NA fleet1     20     4       NA NA           51 derived_quantity
      264           NA fleet1     20     5       NA NA           29 derived_quantity
      265           NA fleet1     20     6       NA NA           12 derived_quantity
      266           NA fleet1     20     7       NA NA           11 derived_quantity
      267           NA fleet1     20     8       NA NA           10 derived_quantity
      268           NA fleet1     20     9       NA NA            5 derived_quantity
      269           NA fleet1     20    10       NA NA            2 derived_quantity
      270           NA fleet1     20    11       NA NA            5 derived_quantity
      271           NA fleet1     20    12       NA NA            2 derived_quantity
      272           NA fleet1     21     1       NA NA           49 derived_quantity
      273           NA fleet1     21     2       NA NA           15 derived_quantity
      274           NA fleet1     21     3       NA NA           38 derived_quantity
      275           NA fleet1     21     4       NA NA           23 derived_quantity
      276           NA fleet1     21     5       NA NA           25 derived_quantity
      277           NA fleet1     21     6       NA NA           17 derived_quantity
      278           NA fleet1     21     7       NA NA           10 derived_quantity
      279           NA fleet1     21     8       NA NA            8 derived_quantity
      280           NA fleet1     21     9       NA NA            2 derived_quantity
      281           NA fleet1     21    10       NA NA            1 derived_quantity
      282           NA fleet1     21    11       NA NA            5 derived_quantity
      283           NA fleet1     21    12       NA NA            7 derived_quantity
      284           NA fleet1     22     1       NA NA           40 derived_quantity
      285           NA fleet1     22     2       NA NA           67 derived_quantity
      286           NA fleet1     22     3       NA NA           14 derived_quantity
      287           NA fleet1     22     4       NA NA           23 derived_quantity
      288           NA fleet1     22     5       NA NA           15 derived_quantity
      289           NA fleet1     22     6       NA NA            6 derived_quantity
      290           NA fleet1     22     7       NA NA           14 derived_quantity
      291           NA fleet1     22     8       NA NA            4 derived_quantity
      292           NA fleet1     22     9       NA NA            5 derived_quantity
      293           NA fleet1     22    10       NA NA            4 derived_quantity
      294           NA fleet1     22    11       NA NA            1 derived_quantity
      295           NA fleet1     22    12       NA NA            7 derived_quantity
      296           NA fleet1     23     1       NA NA           22 derived_quantity
      297           NA fleet1     23     2       NA NA           51 derived_quantity
      298           NA fleet1     23     3       NA NA           56 derived_quantity
      299           NA fleet1     23     4       NA NA           16 derived_quantity
      300           NA fleet1     23     5       NA NA           11 derived_quantity
      301           NA fleet1     23     6       NA NA            7 derived_quantity
      302           NA fleet1     23     7       NA NA           18 derived_quantity
      303           NA fleet1     23     8       NA NA            5 derived_quantity
      304           NA fleet1     23     9       NA NA            1 derived_quantity
      305           NA fleet1     23    10       NA NA            3 derived_quantity
      306           NA fleet1     23    11       NA NA            5 derived_quantity
      307           NA fleet1     23    12       NA NA            5 derived_quantity
      308           NA fleet1     24     1       NA NA           24 derived_quantity
      309           NA fleet1     24     2       NA NA           38 derived_quantity
      310           NA fleet1     24     3       NA NA           48 derived_quantity
      311           NA fleet1     24     4       NA NA           36 derived_quantity
      312           NA fleet1     24     5       NA NA           14 derived_quantity
      313           NA fleet1     24     6       NA NA           15 derived_quantity
      314           NA fleet1     24     7       NA NA            4 derived_quantity
      315           NA fleet1     24     8       NA NA            6 derived_quantity
      316           NA fleet1     24     9       NA NA            8 derived_quantity
      317           NA fleet1     24    10       NA NA            1 derived_quantity
      318           NA fleet1     24    11       NA NA            4 derived_quantity
      319           NA fleet1     24    12       NA NA            2 derived_quantity
      320           NA fleet1     25     1       NA NA           36 derived_quantity
          distribution input_type  lpdf log_sd
          <chr>        <chr>      <dbl>  <dbl>
        1 <NA>         <NA>         NA      NA
        2 <NA>         <NA>         NA      NA
        3 <NA>         <NA>         NA      NA
        4 <NA>         <NA>         NA      NA
        5 <NA>         <NA>         NA      NA
        6 <NA>         <NA>         NA      NA
        7 <NA>         <NA>         NA      NA
        8 <NA>         <NA>         NA      NA
        9 <NA>         <NA>         NA      NA
       10 <NA>         <NA>         NA      NA
       11 <NA>         <NA>         NA      NA
       12 <NA>         <NA>         NA      NA
       13 <NA>         <NA>         NA      NA
       14 <NA>         <NA>         NA      NA
       15 <NA>         <NA>         NA      NA
       16 <NA>         <NA>         NA      NA
       17 <NA>         <NA>         NA      NA
       18 <NA>         <NA>         NA      NA
       19 <NA>         <NA>         NA      NA
       20 <NA>         <NA>         NA      NA
       21 <NA>         <NA>         NA      NA
       22 <NA>         <NA>         NA      NA
       23 <NA>         <NA>         NA      NA
       24 <NA>         <NA>         NA      NA
       25 <NA>         <NA>         NA      NA
       26 <NA>         <NA>         NA      NA
       27 <NA>         <NA>         NA      NA
       28 <NA>         <NA>         NA      NA
       29 <NA>         <NA>         NA      NA
       30 <NA>         <NA>         NA      NA
       31 <NA>         <NA>         NA      NA
       32 multinomial  data       -836.     NA
       33 multinomial  data       -836.     NA
       34 multinomial  data       -836.     NA
       35 multinomial  data       -836.     NA
       36 multinomial  data       -836.     NA
       37 multinomial  data       -836.     NA
       38 multinomial  data       -836.     NA
       39 multinomial  data       -836.     NA
       40 multinomial  data       -836.     NA
       41 multinomial  data       -836.     NA
       42 multinomial  data       -836.     NA
       43 multinomial  data       -836.     NA
       44 multinomial  data       -836.     NA
       45 multinomial  data       -836.     NA
       46 multinomial  data       -836.     NA
       47 multinomial  data       -836.     NA
       48 multinomial  data       -836.     NA
       49 multinomial  data       -836.     NA
       50 multinomial  data       -836.     NA
       51 multinomial  data       -836.     NA
       52 multinomial  data       -836.     NA
       53 multinomial  data       -836.     NA
       54 multinomial  data       -836.     NA
       55 multinomial  data       -836.     NA
       56 multinomial  data       -836.     NA
       57 multinomial  data       -836.     NA
       58 multinomial  data       -836.     NA
       59 multinomial  data       -836.     NA
       60 multinomial  data       -836.     NA
       61 multinomial  data       -836.     NA
       62 multinomial  data       -836.     NA
       63 multinomial  data       -836.     NA
       64 multinomial  data       -836.     NA
       65 multinomial  data       -836.     NA
       66 multinomial  data       -836.     NA
       67 multinomial  data       -836.     NA
       68 multinomial  data       -836.     NA
       69 multinomial  data       -836.     NA
       70 multinomial  data       -836.     NA
       71 multinomial  data       -836.     NA
       72 multinomial  data       -836.     NA
       73 multinomial  data       -836.     NA
       74 multinomial  data       -836.     NA
       75 multinomial  data       -836.     NA
       76 multinomial  data       -836.     NA
       77 multinomial  data       -836.     NA
       78 multinomial  data       -836.     NA
       79 multinomial  data       -836.     NA
       80 multinomial  data       -836.     NA
       81 multinomial  data       -836.     NA
       82 multinomial  data       -836.     NA
       83 multinomial  data       -836.     NA
       84 multinomial  data       -836.     NA
       85 multinomial  data       -836.     NA
       86 multinomial  data       -836.     NA
       87 multinomial  data       -836.     NA
       88 multinomial  data       -836.     NA
       89 multinomial  data       -836.     NA
       90 multinomial  data       -836.     NA
       91 multinomial  data       -836.     NA
       92 multinomial  data       -836.     NA
       93 multinomial  data       -836.     NA
       94 multinomial  data       -836.     NA
       95 multinomial  data       -836.     NA
       96 multinomial  data       -836.     NA
       97 multinomial  data       -836.     NA
       98 multinomial  data       -836.     NA
       99 multinomial  data       -836.     NA
      100 multinomial  data       -836.     NA
      101 multinomial  data       -836.     NA
      102 multinomial  data       -836.     NA
      103 multinomial  data       -836.     NA
      104 multinomial  data       -836.     NA
      105 multinomial  data       -836.     NA
      106 multinomial  data       -836.     NA
      107 multinomial  data       -836.     NA
      108 multinomial  data       -836.     NA
      109 multinomial  data       -836.     NA
      110 multinomial  data       -836.     NA
      111 multinomial  data       -836.     NA
      112 multinomial  data       -836.     NA
      113 multinomial  data       -836.     NA
      114 multinomial  data       -836.     NA
      115 multinomial  data       -836.     NA
      116 multinomial  data       -836.     NA
      117 multinomial  data       -836.     NA
      118 multinomial  data       -836.     NA
      119 multinomial  data       -836.     NA
      120 multinomial  data       -836.     NA
      121 multinomial  data       -836.     NA
      122 multinomial  data       -836.     NA
      123 multinomial  data       -836.     NA
      124 multinomial  data       -836.     NA
      125 multinomial  data       -836.     NA
      126 multinomial  data       -836.     NA
      127 multinomial  data       -836.     NA
      128 multinomial  data       -836.     NA
      129 multinomial  data       -836.     NA
      130 multinomial  data       -836.     NA
      131 multinomial  data       -836.     NA
      132 multinomial  data       -836.     NA
      133 multinomial  data       -836.     NA
      134 multinomial  data       -836.     NA
      135 multinomial  data       -836.     NA
      136 multinomial  data       -836.     NA
      137 multinomial  data       -836.     NA
      138 multinomial  data       -836.     NA
      139 multinomial  data       -836.     NA
      140 multinomial  data       -836.     NA
      141 multinomial  data       -836.     NA
      142 multinomial  data       -836.     NA
      143 multinomial  data       -836.     NA
      144 multinomial  data       -836.     NA
      145 multinomial  data       -836.     NA
      146 multinomial  data       -836.     NA
      147 multinomial  data       -836.     NA
      148 multinomial  data       -836.     NA
      149 multinomial  data       -836.     NA
      150 multinomial  data       -836.     NA
      151 multinomial  data       -836.     NA
      152 multinomial  data       -836.     NA
      153 multinomial  data       -836.     NA
      154 multinomial  data       -836.     NA
      155 multinomial  data       -836.     NA
      156 multinomial  data       -836.     NA
      157 multinomial  data       -836.     NA
      158 multinomial  data       -836.     NA
      159 multinomial  data       -836.     NA
      160 multinomial  data       -836.     NA
      161 multinomial  data       -836.     NA
      162 multinomial  data       -836.     NA
      163 multinomial  data       -836.     NA
      164 multinomial  data       -836.     NA
      165 multinomial  data       -836.     NA
      166 multinomial  data       -836.     NA
      167 multinomial  data       -836.     NA
      168 multinomial  data       -836.     NA
      169 multinomial  data       -836.     NA
      170 multinomial  data       -836.     NA
      171 multinomial  data       -836.     NA
      172 multinomial  data       -836.     NA
      173 multinomial  data       -836.     NA
      174 multinomial  data       -836.     NA
      175 multinomial  data       -836.     NA
      176 multinomial  data       -836.     NA
      177 multinomial  data       -836.     NA
      178 multinomial  data       -836.     NA
      179 multinomial  data       -836.     NA
      180 multinomial  data       -836.     NA
      181 multinomial  data       -836.     NA
      182 multinomial  data       -836.     NA
      183 multinomial  data       -836.     NA
      184 multinomial  data       -836.     NA
      185 multinomial  data       -836.     NA
      186 multinomial  data       -836.     NA
      187 multinomial  data       -836.     NA
      188 multinomial  data       -836.     NA
      189 multinomial  data       -836.     NA
      190 multinomial  data       -836.     NA
      191 multinomial  data       -836.     NA
      192 multinomial  data       -836.     NA
      193 multinomial  data       -836.     NA
      194 multinomial  data       -836.     NA
      195 multinomial  data       -836.     NA
      196 multinomial  data       -836.     NA
      197 multinomial  data       -836.     NA
      198 multinomial  data       -836.     NA
      199 multinomial  data       -836.     NA
      200 multinomial  data       -836.     NA
      201 multinomial  data       -836.     NA
      202 multinomial  data       -836.     NA
      203 multinomial  data       -836.     NA
      204 multinomial  data       -836.     NA
      205 multinomial  data       -836.     NA
      206 multinomial  data       -836.     NA
      207 multinomial  data       -836.     NA
      208 multinomial  data       -836.     NA
      209 multinomial  data       -836.     NA
      210 multinomial  data       -836.     NA
      211 multinomial  data       -836.     NA
      212 multinomial  data       -836.     NA
      213 multinomial  data       -836.     NA
      214 multinomial  data       -836.     NA
      215 multinomial  data       -836.     NA
      216 multinomial  data       -836.     NA
      217 multinomial  data       -836.     NA
      218 multinomial  data       -836.     NA
      219 multinomial  data       -836.     NA
      220 multinomial  data       -836.     NA
      221 multinomial  data       -836.     NA
      222 multinomial  data       -836.     NA
      223 multinomial  data       -836.     NA
      224 multinomial  data       -836.     NA
      225 multinomial  data       -836.     NA
      226 multinomial  data       -836.     NA
      227 multinomial  data       -836.     NA
      228 multinomial  data       -836.     NA
      229 multinomial  data       -836.     NA
      230 multinomial  data       -836.     NA
      231 multinomial  data       -836.     NA
      232 multinomial  data       -836.     NA
      233 multinomial  data       -836.     NA
      234 multinomial  data       -836.     NA
      235 multinomial  data       -836.     NA
      236 multinomial  data       -836.     NA
      237 multinomial  data       -836.     NA
      238 multinomial  data       -836.     NA
      239 multinomial  data       -836.     NA
      240 multinomial  data       -836.     NA
      241 multinomial  data       -836.     NA
      242 multinomial  data       -836.     NA
      243 multinomial  data       -836.     NA
      244 multinomial  data       -836.     NA
      245 multinomial  data       -836.     NA
      246 multinomial  data       -836.     NA
      247 multinomial  data       -836.     NA
      248 multinomial  data       -836.     NA
      249 multinomial  data       -836.     NA
      250 multinomial  data       -836.     NA
      251 multinomial  data       -836.     NA
      252 multinomial  data       -836.     NA
      253 multinomial  data       -836.     NA
      254 multinomial  data       -836.     NA
      255 multinomial  data       -836.     NA
      256 multinomial  data       -836.     NA
      257 multinomial  data       -836.     NA
      258 multinomial  data       -836.     NA
      259 multinomial  data       -836.     NA
      260 multinomial  data       -836.     NA
      261 multinomial  data       -836.     NA
      262 multinomial  data       -836.     NA
      263 multinomial  data       -836.     NA
      264 multinomial  data       -836.     NA
      265 multinomial  data       -836.     NA
      266 multinomial  data       -836.     NA
      267 multinomial  data       -836.     NA
      268 multinomial  data       -836.     NA
      269 multinomial  data       -836.     NA
      270 multinomial  data       -836.     NA
      271 multinomial  data       -836.     NA
      272 multinomial  data       -836.     NA
      273 multinomial  data       -836.     NA
      274 multinomial  data       -836.     NA
      275 multinomial  data       -836.     NA
      276 multinomial  data       -836.     NA
      277 multinomial  data       -836.     NA
      278 multinomial  data       -836.     NA
      279 multinomial  data       -836.     NA
      280 multinomial  data       -836.     NA
      281 multinomial  data       -836.     NA
      282 multinomial  data       -836.     NA
      283 multinomial  data       -836.     NA
      284 multinomial  data       -836.     NA
      285 multinomial  data       -836.     NA
      286 multinomial  data       -836.     NA
      287 multinomial  data       -836.     NA
      288 multinomial  data       -836.     NA
      289 multinomial  data       -836.     NA
      290 multinomial  data       -836.     NA
      291 multinomial  data       -836.     NA
      292 multinomial  data       -836.     NA
      293 multinomial  data       -836.     NA
      294 multinomial  data       -836.     NA
      295 multinomial  data       -836.     NA
      296 multinomial  data       -836.     NA
      297 multinomial  data       -836.     NA
      298 multinomial  data       -836.     NA
      299 multinomial  data       -836.     NA
      300 multinomial  data       -836.     NA
      301 multinomial  data       -836.     NA
      302 multinomial  data       -836.     NA
      303 multinomial  data       -836.     NA
      304 multinomial  data       -836.     NA
      305 multinomial  data       -836.     NA
      306 multinomial  data       -836.     NA
      307 multinomial  data       -836.     NA
      308 multinomial  data       -836.     NA
      309 multinomial  data       -836.     NA
      310 multinomial  data       -836.     NA
      311 multinomial  data       -836.     NA
      312 multinomial  data       -836.     NA
      313 multinomial  data       -836.     NA
      314 multinomial  data       -836.     NA
      315 multinomial  data       -836.     NA
      316 multinomial  data       -836.     NA
      317 multinomial  data       -836.     NA
      318 multinomial  data       -836.     NA
      319 multinomial  data       -836.     NA
      320 multinomial  data       -836.     NA
      # i 13,676 more rows

# `get_estimates()` works with estimation run

    Code
      estimates_snapshot <- dplyr::select(get_estimates(readRDS(fit_files[[1]])),
      -estimated, -expected, -uncertainty, -gradient, -likelihood, -log_like_cv)
      estimates_snapshot_lines <- capture.output(print(estimates_snapshot, n = 320,
        width = Inf))
      cat(trimws(estimates_snapshot_lines, which = "right"), sep = "\n")
    Output
      # A tibble: 13,996 x 18
          module_name module_id module_type label            type   type_id
          <chr>           <int> <chr>       <chr>            <chr>    <int>
        1 Fleet               1 <NA>        log_Fmort        vector       4
        2 Fleet               1 <NA>        log_Fmort        vector       4
        3 Fleet               1 <NA>        log_Fmort        vector       4
        4 Fleet               1 <NA>        log_Fmort        vector       4
        5 Fleet               1 <NA>        log_Fmort        vector       4
        6 Fleet               1 <NA>        log_Fmort        vector       4
        7 Fleet               1 <NA>        log_Fmort        vector       4
        8 Fleet               1 <NA>        log_Fmort        vector       4
        9 Fleet               1 <NA>        log_Fmort        vector       4
       10 Fleet               1 <NA>        log_Fmort        vector       4
       11 Fleet               1 <NA>        log_Fmort        vector       4
       12 Fleet               1 <NA>        log_Fmort        vector       4
       13 Fleet               1 <NA>        log_Fmort        vector       4
       14 Fleet               1 <NA>        log_Fmort        vector       4
       15 Fleet               1 <NA>        log_Fmort        vector       4
       16 Fleet               1 <NA>        log_Fmort        vector       4
       17 Fleet               1 <NA>        log_Fmort        vector       4
       18 Fleet               1 <NA>        log_Fmort        vector       4
       19 Fleet               1 <NA>        log_Fmort        vector       4
       20 Fleet               1 <NA>        log_Fmort        vector       4
       21 Fleet               1 <NA>        log_Fmort        vector       4
       22 Fleet               1 <NA>        log_Fmort        vector       4
       23 Fleet               1 <NA>        log_Fmort        vector       4
       24 Fleet               1 <NA>        log_Fmort        vector       4
       25 Fleet               1 <NA>        log_Fmort        vector       4
       26 Fleet               1 <NA>        log_Fmort        vector       4
       27 Fleet               1 <NA>        log_Fmort        vector       4
       28 Fleet               1 <NA>        log_Fmort        vector       4
       29 Fleet               1 <NA>        log_Fmort        vector       4
       30 Fleet               1 <NA>        log_Fmort        vector       4
       31 Fleet               1 <NA>        log_q            vector       3
       32 Fleet               1 <NA>        agecomp_expected <NA>        NA
       33 Fleet               1 <NA>        agecomp_expected <NA>        NA
       34 Fleet               1 <NA>        agecomp_expected <NA>        NA
       35 Fleet               1 <NA>        agecomp_expected <NA>        NA
       36 Fleet               1 <NA>        agecomp_expected <NA>        NA
       37 Fleet               1 <NA>        agecomp_expected <NA>        NA
       38 Fleet               1 <NA>        agecomp_expected <NA>        NA
       39 Fleet               1 <NA>        agecomp_expected <NA>        NA
       40 Fleet               1 <NA>        agecomp_expected <NA>        NA
       41 Fleet               1 <NA>        agecomp_expected <NA>        NA
       42 Fleet               1 <NA>        agecomp_expected <NA>        NA
       43 Fleet               1 <NA>        agecomp_expected <NA>        NA
       44 Fleet               1 <NA>        agecomp_expected <NA>        NA
       45 Fleet               1 <NA>        agecomp_expected <NA>        NA
       46 Fleet               1 <NA>        agecomp_expected <NA>        NA
       47 Fleet               1 <NA>        agecomp_expected <NA>        NA
       48 Fleet               1 <NA>        agecomp_expected <NA>        NA
       49 Fleet               1 <NA>        agecomp_expected <NA>        NA
       50 Fleet               1 <NA>        agecomp_expected <NA>        NA
       51 Fleet               1 <NA>        agecomp_expected <NA>        NA
       52 Fleet               1 <NA>        agecomp_expected <NA>        NA
       53 Fleet               1 <NA>        agecomp_expected <NA>        NA
       54 Fleet               1 <NA>        agecomp_expected <NA>        NA
       55 Fleet               1 <NA>        agecomp_expected <NA>        NA
       56 Fleet               1 <NA>        agecomp_expected <NA>        NA
       57 Fleet               1 <NA>        agecomp_expected <NA>        NA
       58 Fleet               1 <NA>        agecomp_expected <NA>        NA
       59 Fleet               1 <NA>        agecomp_expected <NA>        NA
       60 Fleet               1 <NA>        agecomp_expected <NA>        NA
       61 Fleet               1 <NA>        agecomp_expected <NA>        NA
       62 Fleet               1 <NA>        agecomp_expected <NA>        NA
       63 Fleet               1 <NA>        agecomp_expected <NA>        NA
       64 Fleet               1 <NA>        agecomp_expected <NA>        NA
       65 Fleet               1 <NA>        agecomp_expected <NA>        NA
       66 Fleet               1 <NA>        agecomp_expected <NA>        NA
       67 Fleet               1 <NA>        agecomp_expected <NA>        NA
       68 Fleet               1 <NA>        agecomp_expected <NA>        NA
       69 Fleet               1 <NA>        agecomp_expected <NA>        NA
       70 Fleet               1 <NA>        agecomp_expected <NA>        NA
       71 Fleet               1 <NA>        agecomp_expected <NA>        NA
       72 Fleet               1 <NA>        agecomp_expected <NA>        NA
       73 Fleet               1 <NA>        agecomp_expected <NA>        NA
       74 Fleet               1 <NA>        agecomp_expected <NA>        NA
       75 Fleet               1 <NA>        agecomp_expected <NA>        NA
       76 Fleet               1 <NA>        agecomp_expected <NA>        NA
       77 Fleet               1 <NA>        agecomp_expected <NA>        NA
       78 Fleet               1 <NA>        agecomp_expected <NA>        NA
       79 Fleet               1 <NA>        agecomp_expected <NA>        NA
       80 Fleet               1 <NA>        agecomp_expected <NA>        NA
       81 Fleet               1 <NA>        agecomp_expected <NA>        NA
       82 Fleet               1 <NA>        agecomp_expected <NA>        NA
       83 Fleet               1 <NA>        agecomp_expected <NA>        NA
       84 Fleet               1 <NA>        agecomp_expected <NA>        NA
       85 Fleet               1 <NA>        agecomp_expected <NA>        NA
       86 Fleet               1 <NA>        agecomp_expected <NA>        NA
       87 Fleet               1 <NA>        agecomp_expected <NA>        NA
       88 Fleet               1 <NA>        agecomp_expected <NA>        NA
       89 Fleet               1 <NA>        agecomp_expected <NA>        NA
       90 Fleet               1 <NA>        agecomp_expected <NA>        NA
       91 Fleet               1 <NA>        agecomp_expected <NA>        NA
       92 Fleet               1 <NA>        agecomp_expected <NA>        NA
       93 Fleet               1 <NA>        agecomp_expected <NA>        NA
       94 Fleet               1 <NA>        agecomp_expected <NA>        NA
       95 Fleet               1 <NA>        agecomp_expected <NA>        NA
       96 Fleet               1 <NA>        agecomp_expected <NA>        NA
       97 Fleet               1 <NA>        agecomp_expected <NA>        NA
       98 Fleet               1 <NA>        agecomp_expected <NA>        NA
       99 Fleet               1 <NA>        agecomp_expected <NA>        NA
      100 Fleet               1 <NA>        agecomp_expected <NA>        NA
      101 Fleet               1 <NA>        agecomp_expected <NA>        NA
      102 Fleet               1 <NA>        agecomp_expected <NA>        NA
      103 Fleet               1 <NA>        agecomp_expected <NA>        NA
      104 Fleet               1 <NA>        agecomp_expected <NA>        NA
      105 Fleet               1 <NA>        agecomp_expected <NA>        NA
      106 Fleet               1 <NA>        agecomp_expected <NA>        NA
      107 Fleet               1 <NA>        agecomp_expected <NA>        NA
      108 Fleet               1 <NA>        agecomp_expected <NA>        NA
      109 Fleet               1 <NA>        agecomp_expected <NA>        NA
      110 Fleet               1 <NA>        agecomp_expected <NA>        NA
      111 Fleet               1 <NA>        agecomp_expected <NA>        NA
      112 Fleet               1 <NA>        agecomp_expected <NA>        NA
      113 Fleet               1 <NA>        agecomp_expected <NA>        NA
      114 Fleet               1 <NA>        agecomp_expected <NA>        NA
      115 Fleet               1 <NA>        agecomp_expected <NA>        NA
      116 Fleet               1 <NA>        agecomp_expected <NA>        NA
      117 Fleet               1 <NA>        agecomp_expected <NA>        NA
      118 Fleet               1 <NA>        agecomp_expected <NA>        NA
      119 Fleet               1 <NA>        agecomp_expected <NA>        NA
      120 Fleet               1 <NA>        agecomp_expected <NA>        NA
      121 Fleet               1 <NA>        agecomp_expected <NA>        NA
      122 Fleet               1 <NA>        agecomp_expected <NA>        NA
      123 Fleet               1 <NA>        agecomp_expected <NA>        NA
      124 Fleet               1 <NA>        agecomp_expected <NA>        NA
      125 Fleet               1 <NA>        agecomp_expected <NA>        NA
      126 Fleet               1 <NA>        agecomp_expected <NA>        NA
      127 Fleet               1 <NA>        agecomp_expected <NA>        NA
      128 Fleet               1 <NA>        agecomp_expected <NA>        NA
      129 Fleet               1 <NA>        agecomp_expected <NA>        NA
      130 Fleet               1 <NA>        agecomp_expected <NA>        NA
      131 Fleet               1 <NA>        agecomp_expected <NA>        NA
      132 Fleet               1 <NA>        agecomp_expected <NA>        NA
      133 Fleet               1 <NA>        agecomp_expected <NA>        NA
      134 Fleet               1 <NA>        agecomp_expected <NA>        NA
      135 Fleet               1 <NA>        agecomp_expected <NA>        NA
      136 Fleet               1 <NA>        agecomp_expected <NA>        NA
      137 Fleet               1 <NA>        agecomp_expected <NA>        NA
      138 Fleet               1 <NA>        agecomp_expected <NA>        NA
      139 Fleet               1 <NA>        agecomp_expected <NA>        NA
      140 Fleet               1 <NA>        agecomp_expected <NA>        NA
      141 Fleet               1 <NA>        agecomp_expected <NA>        NA
      142 Fleet               1 <NA>        agecomp_expected <NA>        NA
      143 Fleet               1 <NA>        agecomp_expected <NA>        NA
      144 Fleet               1 <NA>        agecomp_expected <NA>        NA
      145 Fleet               1 <NA>        agecomp_expected <NA>        NA
      146 Fleet               1 <NA>        agecomp_expected <NA>        NA
      147 Fleet               1 <NA>        agecomp_expected <NA>        NA
      148 Fleet               1 <NA>        agecomp_expected <NA>        NA
      149 Fleet               1 <NA>        agecomp_expected <NA>        NA
      150 Fleet               1 <NA>        agecomp_expected <NA>        NA
      151 Fleet               1 <NA>        agecomp_expected <NA>        NA
      152 Fleet               1 <NA>        agecomp_expected <NA>        NA
      153 Fleet               1 <NA>        agecomp_expected <NA>        NA
      154 Fleet               1 <NA>        agecomp_expected <NA>        NA
      155 Fleet               1 <NA>        agecomp_expected <NA>        NA
      156 Fleet               1 <NA>        agecomp_expected <NA>        NA
      157 Fleet               1 <NA>        agecomp_expected <NA>        NA
      158 Fleet               1 <NA>        agecomp_expected <NA>        NA
      159 Fleet               1 <NA>        agecomp_expected <NA>        NA
      160 Fleet               1 <NA>        agecomp_expected <NA>        NA
      161 Fleet               1 <NA>        agecomp_expected <NA>        NA
      162 Fleet               1 <NA>        agecomp_expected <NA>        NA
      163 Fleet               1 <NA>        agecomp_expected <NA>        NA
      164 Fleet               1 <NA>        agecomp_expected <NA>        NA
      165 Fleet               1 <NA>        agecomp_expected <NA>        NA
      166 Fleet               1 <NA>        agecomp_expected <NA>        NA
      167 Fleet               1 <NA>        agecomp_expected <NA>        NA
      168 Fleet               1 <NA>        agecomp_expected <NA>        NA
      169 Fleet               1 <NA>        agecomp_expected <NA>        NA
      170 Fleet               1 <NA>        agecomp_expected <NA>        NA
      171 Fleet               1 <NA>        agecomp_expected <NA>        NA
      172 Fleet               1 <NA>        agecomp_expected <NA>        NA
      173 Fleet               1 <NA>        agecomp_expected <NA>        NA
      174 Fleet               1 <NA>        agecomp_expected <NA>        NA
      175 Fleet               1 <NA>        agecomp_expected <NA>        NA
      176 Fleet               1 <NA>        agecomp_expected <NA>        NA
      177 Fleet               1 <NA>        agecomp_expected <NA>        NA
      178 Fleet               1 <NA>        agecomp_expected <NA>        NA
      179 Fleet               1 <NA>        agecomp_expected <NA>        NA
      180 Fleet               1 <NA>        agecomp_expected <NA>        NA
      181 Fleet               1 <NA>        agecomp_expected <NA>        NA
      182 Fleet               1 <NA>        agecomp_expected <NA>        NA
      183 Fleet               1 <NA>        agecomp_expected <NA>        NA
      184 Fleet               1 <NA>        agecomp_expected <NA>        NA
      185 Fleet               1 <NA>        agecomp_expected <NA>        NA
      186 Fleet               1 <NA>        agecomp_expected <NA>        NA
      187 Fleet               1 <NA>        agecomp_expected <NA>        NA
      188 Fleet               1 <NA>        agecomp_expected <NA>        NA
      189 Fleet               1 <NA>        agecomp_expected <NA>        NA
      190 Fleet               1 <NA>        agecomp_expected <NA>        NA
      191 Fleet               1 <NA>        agecomp_expected <NA>        NA
      192 Fleet               1 <NA>        agecomp_expected <NA>        NA
      193 Fleet               1 <NA>        agecomp_expected <NA>        NA
      194 Fleet               1 <NA>        agecomp_expected <NA>        NA
      195 Fleet               1 <NA>        agecomp_expected <NA>        NA
      196 Fleet               1 <NA>        agecomp_expected <NA>        NA
      197 Fleet               1 <NA>        agecomp_expected <NA>        NA
      198 Fleet               1 <NA>        agecomp_expected <NA>        NA
      199 Fleet               1 <NA>        agecomp_expected <NA>        NA
      200 Fleet               1 <NA>        agecomp_expected <NA>        NA
      201 Fleet               1 <NA>        agecomp_expected <NA>        NA
      202 Fleet               1 <NA>        agecomp_expected <NA>        NA
      203 Fleet               1 <NA>        agecomp_expected <NA>        NA
      204 Fleet               1 <NA>        agecomp_expected <NA>        NA
      205 Fleet               1 <NA>        agecomp_expected <NA>        NA
      206 Fleet               1 <NA>        agecomp_expected <NA>        NA
      207 Fleet               1 <NA>        agecomp_expected <NA>        NA
      208 Fleet               1 <NA>        agecomp_expected <NA>        NA
      209 Fleet               1 <NA>        agecomp_expected <NA>        NA
      210 Fleet               1 <NA>        agecomp_expected <NA>        NA
      211 Fleet               1 <NA>        agecomp_expected <NA>        NA
      212 Fleet               1 <NA>        agecomp_expected <NA>        NA
      213 Fleet               1 <NA>        agecomp_expected <NA>        NA
      214 Fleet               1 <NA>        agecomp_expected <NA>        NA
      215 Fleet               1 <NA>        agecomp_expected <NA>        NA
      216 Fleet               1 <NA>        agecomp_expected <NA>        NA
      217 Fleet               1 <NA>        agecomp_expected <NA>        NA
      218 Fleet               1 <NA>        agecomp_expected <NA>        NA
      219 Fleet               1 <NA>        agecomp_expected <NA>        NA
      220 Fleet               1 <NA>        agecomp_expected <NA>        NA
      221 Fleet               1 <NA>        agecomp_expected <NA>        NA
      222 Fleet               1 <NA>        agecomp_expected <NA>        NA
      223 Fleet               1 <NA>        agecomp_expected <NA>        NA
      224 Fleet               1 <NA>        agecomp_expected <NA>        NA
      225 Fleet               1 <NA>        agecomp_expected <NA>        NA
      226 Fleet               1 <NA>        agecomp_expected <NA>        NA
      227 Fleet               1 <NA>        agecomp_expected <NA>        NA
      228 Fleet               1 <NA>        agecomp_expected <NA>        NA
      229 Fleet               1 <NA>        agecomp_expected <NA>        NA
      230 Fleet               1 <NA>        agecomp_expected <NA>        NA
      231 Fleet               1 <NA>        agecomp_expected <NA>        NA
      232 Fleet               1 <NA>        agecomp_expected <NA>        NA
      233 Fleet               1 <NA>        agecomp_expected <NA>        NA
      234 Fleet               1 <NA>        agecomp_expected <NA>        NA
      235 Fleet               1 <NA>        agecomp_expected <NA>        NA
      236 Fleet               1 <NA>        agecomp_expected <NA>        NA
      237 Fleet               1 <NA>        agecomp_expected <NA>        NA
      238 Fleet               1 <NA>        agecomp_expected <NA>        NA
      239 Fleet               1 <NA>        agecomp_expected <NA>        NA
      240 Fleet               1 <NA>        agecomp_expected <NA>        NA
      241 Fleet               1 <NA>        agecomp_expected <NA>        NA
      242 Fleet               1 <NA>        agecomp_expected <NA>        NA
      243 Fleet               1 <NA>        agecomp_expected <NA>        NA
      244 Fleet               1 <NA>        agecomp_expected <NA>        NA
      245 Fleet               1 <NA>        agecomp_expected <NA>        NA
      246 Fleet               1 <NA>        agecomp_expected <NA>        NA
      247 Fleet               1 <NA>        agecomp_expected <NA>        NA
      248 Fleet               1 <NA>        agecomp_expected <NA>        NA
      249 Fleet               1 <NA>        agecomp_expected <NA>        NA
      250 Fleet               1 <NA>        agecomp_expected <NA>        NA
      251 Fleet               1 <NA>        agecomp_expected <NA>        NA
      252 Fleet               1 <NA>        agecomp_expected <NA>        NA
      253 Fleet               1 <NA>        agecomp_expected <NA>        NA
      254 Fleet               1 <NA>        agecomp_expected <NA>        NA
      255 Fleet               1 <NA>        agecomp_expected <NA>        NA
      256 Fleet               1 <NA>        agecomp_expected <NA>        NA
      257 Fleet               1 <NA>        agecomp_expected <NA>        NA
      258 Fleet               1 <NA>        agecomp_expected <NA>        NA
      259 Fleet               1 <NA>        agecomp_expected <NA>        NA
      260 Fleet               1 <NA>        agecomp_expected <NA>        NA
      261 Fleet               1 <NA>        agecomp_expected <NA>        NA
      262 Fleet               1 <NA>        agecomp_expected <NA>        NA
      263 Fleet               1 <NA>        agecomp_expected <NA>        NA
      264 Fleet               1 <NA>        agecomp_expected <NA>        NA
      265 Fleet               1 <NA>        agecomp_expected <NA>        NA
      266 Fleet               1 <NA>        agecomp_expected <NA>        NA
      267 Fleet               1 <NA>        agecomp_expected <NA>        NA
      268 Fleet               1 <NA>        agecomp_expected <NA>        NA
      269 Fleet               1 <NA>        agecomp_expected <NA>        NA
      270 Fleet               1 <NA>        agecomp_expected <NA>        NA
      271 Fleet               1 <NA>        agecomp_expected <NA>        NA
      272 Fleet               1 <NA>        agecomp_expected <NA>        NA
      273 Fleet               1 <NA>        agecomp_expected <NA>        NA
      274 Fleet               1 <NA>        agecomp_expected <NA>        NA
      275 Fleet               1 <NA>        agecomp_expected <NA>        NA
      276 Fleet               1 <NA>        agecomp_expected <NA>        NA
      277 Fleet               1 <NA>        agecomp_expected <NA>        NA
      278 Fleet               1 <NA>        agecomp_expected <NA>        NA
      279 Fleet               1 <NA>        agecomp_expected <NA>        NA
      280 Fleet               1 <NA>        agecomp_expected <NA>        NA
      281 Fleet               1 <NA>        agecomp_expected <NA>        NA
      282 Fleet               1 <NA>        agecomp_expected <NA>        NA
      283 Fleet               1 <NA>        agecomp_expected <NA>        NA
      284 Fleet               1 <NA>        agecomp_expected <NA>        NA
      285 Fleet               1 <NA>        agecomp_expected <NA>        NA
      286 Fleet               1 <NA>        agecomp_expected <NA>        NA
      287 Fleet               1 <NA>        agecomp_expected <NA>        NA
      288 Fleet               1 <NA>        agecomp_expected <NA>        NA
      289 Fleet               1 <NA>        agecomp_expected <NA>        NA
      290 Fleet               1 <NA>        agecomp_expected <NA>        NA
      291 Fleet               1 <NA>        agecomp_expected <NA>        NA
      292 Fleet               1 <NA>        agecomp_expected <NA>        NA
      293 Fleet               1 <NA>        agecomp_expected <NA>        NA
      294 Fleet               1 <NA>        agecomp_expected <NA>        NA
      295 Fleet               1 <NA>        agecomp_expected <NA>        NA
      296 Fleet               1 <NA>        agecomp_expected <NA>        NA
      297 Fleet               1 <NA>        agecomp_expected <NA>        NA
      298 Fleet               1 <NA>        agecomp_expected <NA>        NA
      299 Fleet               1 <NA>        agecomp_expected <NA>        NA
      300 Fleet               1 <NA>        agecomp_expected <NA>        NA
      301 Fleet               1 <NA>        agecomp_expected <NA>        NA
      302 Fleet               1 <NA>        agecomp_expected <NA>        NA
      303 Fleet               1 <NA>        agecomp_expected <NA>        NA
      304 Fleet               1 <NA>        agecomp_expected <NA>        NA
      305 Fleet               1 <NA>        agecomp_expected <NA>        NA
      306 Fleet               1 <NA>        agecomp_expected <NA>        NA
      307 Fleet               1 <NA>        agecomp_expected <NA>        NA
      308 Fleet               1 <NA>        agecomp_expected <NA>        NA
      309 Fleet               1 <NA>        agecomp_expected <NA>        NA
      310 Fleet               1 <NA>        agecomp_expected <NA>        NA
      311 Fleet               1 <NA>        agecomp_expected <NA>        NA
      312 Fleet               1 <NA>        agecomp_expected <NA>        NA
      313 Fleet               1 <NA>        agecomp_expected <NA>        NA
      314 Fleet               1 <NA>        agecomp_expected <NA>        NA
      315 Fleet               1 <NA>        agecomp_expected <NA>        NA
      316 Fleet               1 <NA>        agecomp_expected <NA>        NA
      317 Fleet               1 <NA>        agecomp_expected <NA>        NA
      318 Fleet               1 <NA>        agecomp_expected <NA>        NA
      319 Fleet               1 <NA>        agecomp_expected <NA>        NA
      320 Fleet               1 <NA>        agecomp_expected <NA>        NA
          parameter_id fleet  year_i age_i length_i input observed estimation_type
                 <int> <chr>   <int> <int>    <int> <dbl>    <dbl> <chr>
        1            4 fleet1      1    NA       NA -6.99       NA fixed_effects
        2           24 fleet1      2    NA       NA -5.40       NA fixed_effects
        3           25 fleet1      3    NA       NA -4.65       NA fixed_effects
        4           26 fleet1      4    NA       NA -4.19       NA fixed_effects
        5           27 fleet1      5    NA       NA -4.54       NA fixed_effects
        6           28 fleet1      6    NA       NA -3.66       NA fixed_effects
        7           29 fleet1      7    NA       NA -3.64       NA fixed_effects
        8           30 fleet1      8    NA       NA -2.52       NA fixed_effects
        9           31 fleet1      9    NA       NA -3.32       NA fixed_effects
       10           32 fleet1     10    NA       NA -3.03       NA fixed_effects
       11           33 fleet1     11    NA       NA -2.84       NA fixed_effects
       12           34 fleet1     12    NA       NA -2.74       NA fixed_effects
       13           35 fleet1     13    NA       NA -3.22       NA fixed_effects
       14           36 fleet1     14    NA       NA -2.66       NA fixed_effects
       15           37 fleet1     15    NA       NA -2.57       NA fixed_effects
       16           38 fleet1     16    NA       NA -2.74       NA fixed_effects
       17           39 fleet1     17    NA       NA -1.73       NA fixed_effects
       18           40 fleet1     18    NA       NA -2.04       NA fixed_effects
       19           41 fleet1     19    NA       NA -2.05       NA fixed_effects
       20           42 fleet1     20    NA       NA -2.07       NA fixed_effects
       21           43 fleet1     21    NA       NA -1.58       NA fixed_effects
       22           44 fleet1     22    NA       NA -2.06       NA fixed_effects
       23           45 fleet1     23    NA       NA -1.31       NA fixed_effects
       24           46 fleet1     24    NA       NA -1.59       NA fixed_effects
       25           47 fleet1     25    NA       NA -1.60       NA fixed_effects
       26           48 fleet1     26    NA       NA -1.74       NA fixed_effects
       27           49 fleet1     27    NA       NA -1.77       NA fixed_effects
       28           50 fleet1     28    NA       NA -1.26       NA fixed_effects
       29           51 fleet1     29    NA       NA -1.67       NA fixed_effects
       30           52 fleet1     30    NA       NA -1.04       NA fixed_effects
       31            3 fleet1     NA    NA       NA  0          NA constant
       32           NA fleet1      1     1       NA NA          14 derived_quantity
       33           NA fleet1      1     2       NA NA          20 derived_quantity
       34           NA fleet1      1     3       NA NA          23 derived_quantity
       35           NA fleet1      1     4       NA NA          30 derived_quantity
       36           NA fleet1      1     5       NA NA          20 derived_quantity
       37           NA fleet1      1     6       NA NA          10 derived_quantity
       38           NA fleet1      1     7       NA NA          22 derived_quantity
       39           NA fleet1      1     8       NA NA          11 derived_quantity
       40           NA fleet1      1     9       NA NA          12 derived_quantity
       41           NA fleet1      1    10       NA NA           6 derived_quantity
       42           NA fleet1      1    11       NA NA           7 derived_quantity
       43           NA fleet1      1    12       NA NA          25 derived_quantity
       44           NA fleet1      2     1       NA NA          21 derived_quantity
       45           NA fleet1      2     2       NA NA          19 derived_quantity
       46           NA fleet1      2     3       NA NA          22 derived_quantity
       47           NA fleet1      2     4       NA NA          18 derived_quantity
       48           NA fleet1      2     5       NA NA          31 derived_quantity
       49           NA fleet1      2     6       NA NA          11 derived_quantity
       50           NA fleet1      2     7       NA NA          13 derived_quantity
       51           NA fleet1      2     8       NA NA          17 derived_quantity
       52           NA fleet1      2     9       NA NA           9 derived_quantity
       53           NA fleet1      2    10       NA NA           7 derived_quantity
       54           NA fleet1      2    11       NA NA           5 derived_quantity
       55           NA fleet1      2    12       NA NA          27 derived_quantity
       56           NA fleet1      3     1       NA NA          11 derived_quantity
       57           NA fleet1      3     2       NA NA          30 derived_quantity
       58           NA fleet1      3     3       NA NA          21 derived_quantity
       59           NA fleet1      3     4       NA NA          23 derived_quantity
       60           NA fleet1      3     5       NA NA          19 derived_quantity
       61           NA fleet1      3     6       NA NA          18 derived_quantity
       62           NA fleet1      3     7       NA NA          14 derived_quantity
       63           NA fleet1      3     8       NA NA          17 derived_quantity
       64           NA fleet1      3     9       NA NA           6 derived_quantity
       65           NA fleet1      3    10       NA NA           9 derived_quantity
       66           NA fleet1      3    11       NA NA           6 derived_quantity
       67           NA fleet1      3    12       NA NA          26 derived_quantity
       68           NA fleet1      4     1       NA NA          10 derived_quantity
       69           NA fleet1      4     2       NA NA          11 derived_quantity
       70           NA fleet1      4     3       NA NA          44 derived_quantity
       71           NA fleet1      4     4       NA NA          20 derived_quantity
       72           NA fleet1      4     5       NA NA          20 derived_quantity
       73           NA fleet1      4     6       NA NA          21 derived_quantity
       74           NA fleet1      4     7       NA NA          13 derived_quantity
       75           NA fleet1      4     8       NA NA          11 derived_quantity
       76           NA fleet1      4     9       NA NA          13 derived_quantity
       77           NA fleet1      4    10       NA NA           6 derived_quantity
       78           NA fleet1      4    11       NA NA           3 derived_quantity
       79           NA fleet1      4    12       NA NA          28 derived_quantity
       80           NA fleet1      5     1       NA NA          31 derived_quantity
       81           NA fleet1      5     2       NA NA          10 derived_quantity
       82           NA fleet1      5     3       NA NA          32 derived_quantity
       83           NA fleet1      5     4       NA NA          34 derived_quantity
       84           NA fleet1      5     5       NA NA          18 derived_quantity
       85           NA fleet1      5     6       NA NA          19 derived_quantity
       86           NA fleet1      5     7       NA NA           9 derived_quantity
       87           NA fleet1      5     8       NA NA           9 derived_quantity
       88           NA fleet1      5     9       NA NA           6 derived_quantity
       89           NA fleet1      5    10       NA NA           4 derived_quantity
       90           NA fleet1      5    11       NA NA           6 derived_quantity
       91           NA fleet1      5    12       NA NA          22 derived_quantity
       92           NA fleet1      6     1       NA NA          16 derived_quantity
       93           NA fleet1      6     2       NA NA          37 derived_quantity
       94           NA fleet1      6     3       NA NA          14 derived_quantity
       95           NA fleet1      6     4       NA NA          18 derived_quantity
       96           NA fleet1      6     5       NA NA          29 derived_quantity
       97           NA fleet1      6     6       NA NA          15 derived_quantity
       98           NA fleet1      6     7       NA NA          12 derived_quantity
       99           NA fleet1      6     8       NA NA          13 derived_quantity
      100           NA fleet1      6     9       NA NA           8 derived_quantity
      101           NA fleet1      6    10       NA NA           9 derived_quantity
      102           NA fleet1      6    11       NA NA           5 derived_quantity
      103           NA fleet1      6    12       NA NA          24 derived_quantity
      104           NA fleet1      7     1       NA NA          11 derived_quantity
      105           NA fleet1      7     2       NA NA          26 derived_quantity
      106           NA fleet1      7     3       NA NA          44 derived_quantity
      107           NA fleet1      7     4       NA NA          17 derived_quantity
      108           NA fleet1      7     5       NA NA          19 derived_quantity
      109           NA fleet1      7     6       NA NA          28 derived_quantity
      110           NA fleet1      7     7       NA NA           7 derived_quantity
      111           NA fleet1      7     8       NA NA           8 derived_quantity
      112           NA fleet1      7     9       NA NA           7 derived_quantity
      113           NA fleet1      7    10       NA NA           4 derived_quantity
      114           NA fleet1      7    11       NA NA           5 derived_quantity
      115           NA fleet1      7    12       NA NA          24 derived_quantity
      116           NA fleet1      8     1       NA NA          18 derived_quantity
      117           NA fleet1      8     2       NA NA          15 derived_quantity
      118           NA fleet1      8     3       NA NA          44 derived_quantity
      119           NA fleet1      8     4       NA NA          40 derived_quantity
      120           NA fleet1      8     5       NA NA          12 derived_quantity
      121           NA fleet1      8     6       NA NA           8 derived_quantity
      122           NA fleet1      8     7       NA NA          14 derived_quantity
      123           NA fleet1      8     8       NA NA          13 derived_quantity
      124           NA fleet1      8     9       NA NA           5 derived_quantity
      125           NA fleet1      8    10       NA NA           5 derived_quantity
      126           NA fleet1      8    11       NA NA           5 derived_quantity
      127           NA fleet1      8    12       NA NA          21 derived_quantity
      128           NA fleet1      9     1       NA NA          14 derived_quantity
      129           NA fleet1      9     2       NA NA          32 derived_quantity
      130           NA fleet1      9     3       NA NA          27 derived_quantity
      131           NA fleet1      9     4       NA NA          35 derived_quantity
      132           NA fleet1      9     5       NA NA          31 derived_quantity
      133           NA fleet1      9     6       NA NA           9 derived_quantity
      134           NA fleet1      9     7       NA NA           9 derived_quantity
      135           NA fleet1      9     8       NA NA          12 derived_quantity
      136           NA fleet1      9     9       NA NA           5 derived_quantity
      137           NA fleet1      9    10       NA NA           5 derived_quantity
      138           NA fleet1      9    11       NA NA           3 derived_quantity
      139           NA fleet1      9    12       NA NA          18 derived_quantity
      140           NA fleet1     10     1       NA NA          15 derived_quantity
      141           NA fleet1     10     2       NA NA          21 derived_quantity
      142           NA fleet1     10     3       NA NA          43 derived_quantity
      143           NA fleet1     10     4       NA NA          20 derived_quantity
      144           NA fleet1     10     5       NA NA          22 derived_quantity
      145           NA fleet1     10     6       NA NA          21 derived_quantity
      146           NA fleet1     10     7       NA NA           7 derived_quantity
      147           NA fleet1     10     8       NA NA           8 derived_quantity
      148           NA fleet1     10     9       NA NA          13 derived_quantity
      149           NA fleet1     10    10       NA NA           4 derived_quantity
      150           NA fleet1     10    11       NA NA           4 derived_quantity
      151           NA fleet1     10    12       NA NA          22 derived_quantity
      152           NA fleet1     11     1       NA NA          15 derived_quantity
      153           NA fleet1     11     2       NA NA          34 derived_quantity
      154           NA fleet1     11     3       NA NA          23 derived_quantity
      155           NA fleet1     11     4       NA NA          43 derived_quantity
      156           NA fleet1     11     5       NA NA          15 derived_quantity
      157           NA fleet1     11     6       NA NA          23 derived_quantity
      158           NA fleet1     11     7       NA NA          21 derived_quantity
      159           NA fleet1     11     8       NA NA           5 derived_quantity
      160           NA fleet1     11     9       NA NA           3 derived_quantity
      161           NA fleet1     11    10       NA NA           5 derived_quantity
      162           NA fleet1     11    11       NA NA           2 derived_quantity
      163           NA fleet1     11    12       NA NA          11 derived_quantity
      164           NA fleet1     12     1       NA NA          11 derived_quantity
      165           NA fleet1     12     2       NA NA          32 derived_quantity
      166           NA fleet1     12     3       NA NA          30 derived_quantity
      167           NA fleet1     12     4       NA NA          26 derived_quantity
      168           NA fleet1     12     5       NA NA          33 derived_quantity
      169           NA fleet1     12     6       NA NA          14 derived_quantity
      170           NA fleet1     12     7       NA NA          13 derived_quantity
      171           NA fleet1     12     8       NA NA          21 derived_quantity
      172           NA fleet1     12     9       NA NA           4 derived_quantity
      173           NA fleet1     12    10       NA NA           3 derived_quantity
      174           NA fleet1     12    11       NA NA           4 derived_quantity
      175           NA fleet1     12    12       NA NA           9 derived_quantity
      176           NA fleet1     13     1       NA NA          22 derived_quantity
      177           NA fleet1     13     2       NA NA          21 derived_quantity
      178           NA fleet1     13     3       NA NA          23 derived_quantity
      179           NA fleet1     13     4       NA NA          28 derived_quantity
      180           NA fleet1     13     5       NA NA          23 derived_quantity
      181           NA fleet1     13     6       NA NA          18 derived_quantity
      182           NA fleet1     13     7       NA NA          15 derived_quantity
      183           NA fleet1     13     8       NA NA          18 derived_quantity
      184           NA fleet1     13     9       NA NA          16 derived_quantity
      185           NA fleet1     13    10       NA NA           3 derived_quantity
      186           NA fleet1     13    11       NA NA           3 derived_quantity
      187           NA fleet1     13    12       NA NA          10 derived_quantity
      188           NA fleet1     14     1       NA NA          23 derived_quantity
      189           NA fleet1     14     2       NA NA          28 derived_quantity
      190           NA fleet1     14     3       NA NA          17 derived_quantity
      191           NA fleet1     14     4       NA NA          41 derived_quantity
      192           NA fleet1     14     5       NA NA          24 derived_quantity
      193           NA fleet1     14     6       NA NA          10 derived_quantity
      194           NA fleet1     14     7       NA NA          10 derived_quantity
      195           NA fleet1     14     8       NA NA           8 derived_quantity
      196           NA fleet1     14     9       NA NA           9 derived_quantity
      197           NA fleet1     14    10       NA NA          12 derived_quantity
      198           NA fleet1     14    11       NA NA           5 derived_quantity
      199           NA fleet1     14    12       NA NA          13 derived_quantity
      200           NA fleet1     15     1       NA NA          14 derived_quantity
      201           NA fleet1     15     2       NA NA          38 derived_quantity
      202           NA fleet1     15     3       NA NA          39 derived_quantity
      203           NA fleet1     15     4       NA NA          16 derived_quantity
      204           NA fleet1     15     5       NA NA          28 derived_quantity
      205           NA fleet1     15     6       NA NA          21 derived_quantity
      206           NA fleet1     15     7       NA NA          10 derived_quantity
      207           NA fleet1     15     8       NA NA          11 derived_quantity
      208           NA fleet1     15     9       NA NA           5 derived_quantity
      209           NA fleet1     15    10       NA NA           4 derived_quantity
      210           NA fleet1     15    11       NA NA           0 derived_quantity
      211           NA fleet1     15    12       NA NA          14 derived_quantity
      212           NA fleet1     16     1       NA NA          29 derived_quantity
      213           NA fleet1     16     2       NA NA          25 derived_quantity
      214           NA fleet1     16     3       NA NA          39 derived_quantity
      215           NA fleet1     16     4       NA NA          25 derived_quantity
      216           NA fleet1     16     5       NA NA          17 derived_quantity
      217           NA fleet1     16     6       NA NA          13 derived_quantity
      218           NA fleet1     16     7       NA NA          12 derived_quantity
      219           NA fleet1     16     8       NA NA           8 derived_quantity
      220           NA fleet1     16     9       NA NA           7 derived_quantity
      221           NA fleet1     16    10       NA NA           3 derived_quantity
      222           NA fleet1     16    11       NA NA           7 derived_quantity
      223           NA fleet1     16    12       NA NA          15 derived_quantity
      224           NA fleet1     17     1       NA NA          33 derived_quantity
      225           NA fleet1     17     2       NA NA          38 derived_quantity
      226           NA fleet1     17     3       NA NA          28 derived_quantity
      227           NA fleet1     17     4       NA NA          29 derived_quantity
      228           NA fleet1     17     5       NA NA          24 derived_quantity
      229           NA fleet1     17     6       NA NA           6 derived_quantity
      230           NA fleet1     17     7       NA NA          10 derived_quantity
      231           NA fleet1     17     8       NA NA           1 derived_quantity
      232           NA fleet1     17     9       NA NA           6 derived_quantity
      233           NA fleet1     17    10       NA NA           5 derived_quantity
      234           NA fleet1     17    11       NA NA           2 derived_quantity
      235           NA fleet1     17    12       NA NA          18 derived_quantity
      236           NA fleet1     18     1       NA NA          17 derived_quantity
      237           NA fleet1     18     2       NA NA          40 derived_quantity
      238           NA fleet1     18     3       NA NA          50 derived_quantity
      239           NA fleet1     18     4       NA NA          25 derived_quantity
      240           NA fleet1     18     5       NA NA          17 derived_quantity
      241           NA fleet1     18     6       NA NA          13 derived_quantity
      242           NA fleet1     18     7       NA NA          11 derived_quantity
      243           NA fleet1     18     8       NA NA           8 derived_quantity
      244           NA fleet1     18     9       NA NA           7 derived_quantity
      245           NA fleet1     18    10       NA NA           1 derived_quantity
      246           NA fleet1     18    11       NA NA           4 derived_quantity
      247           NA fleet1     18    12       NA NA           7 derived_quantity
      248           NA fleet1     19     1       NA NA          26 derived_quantity
      249           NA fleet1     19     2       NA NA          24 derived_quantity
      250           NA fleet1     19     3       NA NA          37 derived_quantity
      251           NA fleet1     19     4       NA NA          34 derived_quantity
      252           NA fleet1     19     5       NA NA          22 derived_quantity
      253           NA fleet1     19     6       NA NA          23 derived_quantity
      254           NA fleet1     19     7       NA NA           8 derived_quantity
      255           NA fleet1     19     8       NA NA           6 derived_quantity
      256           NA fleet1     19     9       NA NA           3 derived_quantity
      257           NA fleet1     19    10       NA NA           5 derived_quantity
      258           NA fleet1     19    11       NA NA           4 derived_quantity
      259           NA fleet1     19    12       NA NA           8 derived_quantity
      260           NA fleet1     20     1       NA NA           9 derived_quantity
      261           NA fleet1     20     2       NA NA          31 derived_quantity
      262           NA fleet1     20     3       NA NA          33 derived_quantity
      263           NA fleet1     20     4       NA NA          51 derived_quantity
      264           NA fleet1     20     5       NA NA          29 derived_quantity
      265           NA fleet1     20     6       NA NA          12 derived_quantity
      266           NA fleet1     20     7       NA NA          11 derived_quantity
      267           NA fleet1     20     8       NA NA          10 derived_quantity
      268           NA fleet1     20     9       NA NA           5 derived_quantity
      269           NA fleet1     20    10       NA NA           2 derived_quantity
      270           NA fleet1     20    11       NA NA           5 derived_quantity
      271           NA fleet1     20    12       NA NA           2 derived_quantity
      272           NA fleet1     21     1       NA NA          49 derived_quantity
      273           NA fleet1     21     2       NA NA          15 derived_quantity
      274           NA fleet1     21     3       NA NA          38 derived_quantity
      275           NA fleet1     21     4       NA NA          23 derived_quantity
      276           NA fleet1     21     5       NA NA          25 derived_quantity
      277           NA fleet1     21     6       NA NA          17 derived_quantity
      278           NA fleet1     21     7       NA NA          10 derived_quantity
      279           NA fleet1     21     8       NA NA           8 derived_quantity
      280           NA fleet1     21     9       NA NA           2 derived_quantity
      281           NA fleet1     21    10       NA NA           1 derived_quantity
      282           NA fleet1     21    11       NA NA           5 derived_quantity
      283           NA fleet1     21    12       NA NA           7 derived_quantity
      284           NA fleet1     22     1       NA NA          40 derived_quantity
      285           NA fleet1     22     2       NA NA          67 derived_quantity
      286           NA fleet1     22     3       NA NA          14 derived_quantity
      287           NA fleet1     22     4       NA NA          23 derived_quantity
      288           NA fleet1     22     5       NA NA          15 derived_quantity
      289           NA fleet1     22     6       NA NA           6 derived_quantity
      290           NA fleet1     22     7       NA NA          14 derived_quantity
      291           NA fleet1     22     8       NA NA           4 derived_quantity
      292           NA fleet1     22     9       NA NA           5 derived_quantity
      293           NA fleet1     22    10       NA NA           4 derived_quantity
      294           NA fleet1     22    11       NA NA           1 derived_quantity
      295           NA fleet1     22    12       NA NA           7 derived_quantity
      296           NA fleet1     23     1       NA NA          22 derived_quantity
      297           NA fleet1     23     2       NA NA          51 derived_quantity
      298           NA fleet1     23     3       NA NA          56 derived_quantity
      299           NA fleet1     23     4       NA NA          16 derived_quantity
      300           NA fleet1     23     5       NA NA          11 derived_quantity
      301           NA fleet1     23     6       NA NA           7 derived_quantity
      302           NA fleet1     23     7       NA NA          18 derived_quantity
      303           NA fleet1     23     8       NA NA           5 derived_quantity
      304           NA fleet1     23     9       NA NA           1 derived_quantity
      305           NA fleet1     23    10       NA NA           3 derived_quantity
      306           NA fleet1     23    11       NA NA           5 derived_quantity
      307           NA fleet1     23    12       NA NA           5 derived_quantity
      308           NA fleet1     24     1       NA NA          24 derived_quantity
      309           NA fleet1     24     2       NA NA          38 derived_quantity
      310           NA fleet1     24     3       NA NA          48 derived_quantity
      311           NA fleet1     24     4       NA NA          36 derived_quantity
      312           NA fleet1     24     5       NA NA          14 derived_quantity
      313           NA fleet1     24     6       NA NA          15 derived_quantity
      314           NA fleet1     24     7       NA NA           4 derived_quantity
      315           NA fleet1     24     8       NA NA           6 derived_quantity
      316           NA fleet1     24     9       NA NA           8 derived_quantity
      317           NA fleet1     24    10       NA NA           1 derived_quantity
      318           NA fleet1     24    11       NA NA           4 derived_quantity
      319           NA fleet1     24    12       NA NA           2 derived_quantity
      320           NA fleet1     25     1       NA NA          36 derived_quantity
          distribution input_type  lpdf log_sd
          <chr>        <chr>      <dbl>  <dbl>
        1 <NA>         <NA>         NA      NA
        2 <NA>         <NA>         NA      NA
        3 <NA>         <NA>         NA      NA
        4 <NA>         <NA>         NA      NA
        5 <NA>         <NA>         NA      NA
        6 <NA>         <NA>         NA      NA
        7 <NA>         <NA>         NA      NA
        8 <NA>         <NA>         NA      NA
        9 <NA>         <NA>         NA      NA
       10 <NA>         <NA>         NA      NA
       11 <NA>         <NA>         NA      NA
       12 <NA>         <NA>         NA      NA
       13 <NA>         <NA>         NA      NA
       14 <NA>         <NA>         NA      NA
       15 <NA>         <NA>         NA      NA
       16 <NA>         <NA>         NA      NA
       17 <NA>         <NA>         NA      NA
       18 <NA>         <NA>         NA      NA
       19 <NA>         <NA>         NA      NA
       20 <NA>         <NA>         NA      NA
       21 <NA>         <NA>         NA      NA
       22 <NA>         <NA>         NA      NA
       23 <NA>         <NA>         NA      NA
       24 <NA>         <NA>         NA      NA
       25 <NA>         <NA>         NA      NA
       26 <NA>         <NA>         NA      NA
       27 <NA>         <NA>         NA      NA
       28 <NA>         <NA>         NA      NA
       29 <NA>         <NA>         NA      NA
       30 <NA>         <NA>         NA      NA
       31 <NA>         <NA>         NA      NA
       32 multinomial  data       -826.     NA
       33 multinomial  data       -826.     NA
       34 multinomial  data       -826.     NA
       35 multinomial  data       -826.     NA
       36 multinomial  data       -826.     NA
       37 multinomial  data       -826.     NA
       38 multinomial  data       -826.     NA
       39 multinomial  data       -826.     NA
       40 multinomial  data       -826.     NA
       41 multinomial  data       -826.     NA
       42 multinomial  data       -826.     NA
       43 multinomial  data       -826.     NA
       44 multinomial  data       -826.     NA
       45 multinomial  data       -826.     NA
       46 multinomial  data       -826.     NA
       47 multinomial  data       -826.     NA
       48 multinomial  data       -826.     NA
       49 multinomial  data       -826.     NA
       50 multinomial  data       -826.     NA
       51 multinomial  data       -826.     NA
       52 multinomial  data       -826.     NA
       53 multinomial  data       -826.     NA
       54 multinomial  data       -826.     NA
       55 multinomial  data       -826.     NA
       56 multinomial  data       -826.     NA
       57 multinomial  data       -826.     NA
       58 multinomial  data       -826.     NA
       59 multinomial  data       -826.     NA
       60 multinomial  data       -826.     NA
       61 multinomial  data       -826.     NA
       62 multinomial  data       -826.     NA
       63 multinomial  data       -826.     NA
       64 multinomial  data       -826.     NA
       65 multinomial  data       -826.     NA
       66 multinomial  data       -826.     NA
       67 multinomial  data       -826.     NA
       68 multinomial  data       -826.     NA
       69 multinomial  data       -826.     NA
       70 multinomial  data       -826.     NA
       71 multinomial  data       -826.     NA
       72 multinomial  data       -826.     NA
       73 multinomial  data       -826.     NA
       74 multinomial  data       -826.     NA
       75 multinomial  data       -826.     NA
       76 multinomial  data       -826.     NA
       77 multinomial  data       -826.     NA
       78 multinomial  data       -826.     NA
       79 multinomial  data       -826.     NA
       80 multinomial  data       -826.     NA
       81 multinomial  data       -826.     NA
       82 multinomial  data       -826.     NA
       83 multinomial  data       -826.     NA
       84 multinomial  data       -826.     NA
       85 multinomial  data       -826.     NA
       86 multinomial  data       -826.     NA
       87 multinomial  data       -826.     NA
       88 multinomial  data       -826.     NA
       89 multinomial  data       -826.     NA
       90 multinomial  data       -826.     NA
       91 multinomial  data       -826.     NA
       92 multinomial  data       -826.     NA
       93 multinomial  data       -826.     NA
       94 multinomial  data       -826.     NA
       95 multinomial  data       -826.     NA
       96 multinomial  data       -826.     NA
       97 multinomial  data       -826.     NA
       98 multinomial  data       -826.     NA
       99 multinomial  data       -826.     NA
      100 multinomial  data       -826.     NA
      101 multinomial  data       -826.     NA
      102 multinomial  data       -826.     NA
      103 multinomial  data       -826.     NA
      104 multinomial  data       -826.     NA
      105 multinomial  data       -826.     NA
      106 multinomial  data       -826.     NA
      107 multinomial  data       -826.     NA
      108 multinomial  data       -826.     NA
      109 multinomial  data       -826.     NA
      110 multinomial  data       -826.     NA
      111 multinomial  data       -826.     NA
      112 multinomial  data       -826.     NA
      113 multinomial  data       -826.     NA
      114 multinomial  data       -826.     NA
      115 multinomial  data       -826.     NA
      116 multinomial  data       -826.     NA
      117 multinomial  data       -826.     NA
      118 multinomial  data       -826.     NA
      119 multinomial  data       -826.     NA
      120 multinomial  data       -826.     NA
      121 multinomial  data       -826.     NA
      122 multinomial  data       -826.     NA
      123 multinomial  data       -826.     NA
      124 multinomial  data       -826.     NA
      125 multinomial  data       -826.     NA
      126 multinomial  data       -826.     NA
      127 multinomial  data       -826.     NA
      128 multinomial  data       -826.     NA
      129 multinomial  data       -826.     NA
      130 multinomial  data       -826.     NA
      131 multinomial  data       -826.     NA
      132 multinomial  data       -826.     NA
      133 multinomial  data       -826.     NA
      134 multinomial  data       -826.     NA
      135 multinomial  data       -826.     NA
      136 multinomial  data       -826.     NA
      137 multinomial  data       -826.     NA
      138 multinomial  data       -826.     NA
      139 multinomial  data       -826.     NA
      140 multinomial  data       -826.     NA
      141 multinomial  data       -826.     NA
      142 multinomial  data       -826.     NA
      143 multinomial  data       -826.     NA
      144 multinomial  data       -826.     NA
      145 multinomial  data       -826.     NA
      146 multinomial  data       -826.     NA
      147 multinomial  data       -826.     NA
      148 multinomial  data       -826.     NA
      149 multinomial  data       -826.     NA
      150 multinomial  data       -826.     NA
      151 multinomial  data       -826.     NA
      152 multinomial  data       -826.     NA
      153 multinomial  data       -826.     NA
      154 multinomial  data       -826.     NA
      155 multinomial  data       -826.     NA
      156 multinomial  data       -826.     NA
      157 multinomial  data       -826.     NA
      158 multinomial  data       -826.     NA
      159 multinomial  data       -826.     NA
      160 multinomial  data       -826.     NA
      161 multinomial  data       -826.     NA
      162 multinomial  data       -826.     NA
      163 multinomial  data       -826.     NA
      164 multinomial  data       -826.     NA
      165 multinomial  data       -826.     NA
      166 multinomial  data       -826.     NA
      167 multinomial  data       -826.     NA
      168 multinomial  data       -826.     NA
      169 multinomial  data       -826.     NA
      170 multinomial  data       -826.     NA
      171 multinomial  data       -826.     NA
      172 multinomial  data       -826.     NA
      173 multinomial  data       -826.     NA
      174 multinomial  data       -826.     NA
      175 multinomial  data       -826.     NA
      176 multinomial  data       -826.     NA
      177 multinomial  data       -826.     NA
      178 multinomial  data       -826.     NA
      179 multinomial  data       -826.     NA
      180 multinomial  data       -826.     NA
      181 multinomial  data       -826.     NA
      182 multinomial  data       -826.     NA
      183 multinomial  data       -826.     NA
      184 multinomial  data       -826.     NA
      185 multinomial  data       -826.     NA
      186 multinomial  data       -826.     NA
      187 multinomial  data       -826.     NA
      188 multinomial  data       -826.     NA
      189 multinomial  data       -826.     NA
      190 multinomial  data       -826.     NA
      191 multinomial  data       -826.     NA
      192 multinomial  data       -826.     NA
      193 multinomial  data       -826.     NA
      194 multinomial  data       -826.     NA
      195 multinomial  data       -826.     NA
      196 multinomial  data       -826.     NA
      197 multinomial  data       -826.     NA
      198 multinomial  data       -826.     NA
      199 multinomial  data       -826.     NA
      200 multinomial  data       -826.     NA
      201 multinomial  data       -826.     NA
      202 multinomial  data       -826.     NA
      203 multinomial  data       -826.     NA
      204 multinomial  data       -826.     NA
      205 multinomial  data       -826.     NA
      206 multinomial  data       -826.     NA
      207 multinomial  data       -826.     NA
      208 multinomial  data       -826.     NA
      209 multinomial  data       -826.     NA
      210 multinomial  data       -826.     NA
      211 multinomial  data       -826.     NA
      212 multinomial  data       -826.     NA
      213 multinomial  data       -826.     NA
      214 multinomial  data       -826.     NA
      215 multinomial  data       -826.     NA
      216 multinomial  data       -826.     NA
      217 multinomial  data       -826.     NA
      218 multinomial  data       -826.     NA
      219 multinomial  data       -826.     NA
      220 multinomial  data       -826.     NA
      221 multinomial  data       -826.     NA
      222 multinomial  data       -826.     NA
      223 multinomial  data       -826.     NA
      224 multinomial  data       -826.     NA
      225 multinomial  data       -826.     NA
      226 multinomial  data       -826.     NA
      227 multinomial  data       -826.     NA
      228 multinomial  data       -826.     NA
      229 multinomial  data       -826.     NA
      230 multinomial  data       -826.     NA
      231 multinomial  data       -826.     NA
      232 multinomial  data       -826.     NA
      233 multinomial  data       -826.     NA
      234 multinomial  data       -826.     NA
      235 multinomial  data       -826.     NA
      236 multinomial  data       -826.     NA
      237 multinomial  data       -826.     NA
      238 multinomial  data       -826.     NA
      239 multinomial  data       -826.     NA
      240 multinomial  data       -826.     NA
      241 multinomial  data       -826.     NA
      242 multinomial  data       -826.     NA
      243 multinomial  data       -826.     NA
      244 multinomial  data       -826.     NA
      245 multinomial  data       -826.     NA
      246 multinomial  data       -826.     NA
      247 multinomial  data       -826.     NA
      248 multinomial  data       -826.     NA
      249 multinomial  data       -826.     NA
      250 multinomial  data       -826.     NA
      251 multinomial  data       -826.     NA
      252 multinomial  data       -826.     NA
      253 multinomial  data       -826.     NA
      254 multinomial  data       -826.     NA
      255 multinomial  data       -826.     NA
      256 multinomial  data       -826.     NA
      257 multinomial  data       -826.     NA
      258 multinomial  data       -826.     NA
      259 multinomial  data       -826.     NA
      260 multinomial  data       -826.     NA
      261 multinomial  data       -826.     NA
      262 multinomial  data       -826.     NA
      263 multinomial  data       -826.     NA
      264 multinomial  data       -826.     NA
      265 multinomial  data       -826.     NA
      266 multinomial  data       -826.     NA
      267 multinomial  data       -826.     NA
      268 multinomial  data       -826.     NA
      269 multinomial  data       -826.     NA
      270 multinomial  data       -826.     NA
      271 multinomial  data       -826.     NA
      272 multinomial  data       -826.     NA
      273 multinomial  data       -826.     NA
      274 multinomial  data       -826.     NA
      275 multinomial  data       -826.     NA
      276 multinomial  data       -826.     NA
      277 multinomial  data       -826.     NA
      278 multinomial  data       -826.     NA
      279 multinomial  data       -826.     NA
      280 multinomial  data       -826.     NA
      281 multinomial  data       -826.     NA
      282 multinomial  data       -826.     NA
      283 multinomial  data       -826.     NA
      284 multinomial  data       -826.     NA
      285 multinomial  data       -826.     NA
      286 multinomial  data       -826.     NA
      287 multinomial  data       -826.     NA
      288 multinomial  data       -826.     NA
      289 multinomial  data       -826.     NA
      290 multinomial  data       -826.     NA
      291 multinomial  data       -826.     NA
      292 multinomial  data       -826.     NA
      293 multinomial  data       -826.     NA
      294 multinomial  data       -826.     NA
      295 multinomial  data       -826.     NA
      296 multinomial  data       -826.     NA
      297 multinomial  data       -826.     NA
      298 multinomial  data       -826.     NA
      299 multinomial  data       -826.     NA
      300 multinomial  data       -826.     NA
      301 multinomial  data       -826.     NA
      302 multinomial  data       -826.     NA
      303 multinomial  data       -826.     NA
      304 multinomial  data       -826.     NA
      305 multinomial  data       -826.     NA
      306 multinomial  data       -826.     NA
      307 multinomial  data       -826.     NA
      308 multinomial  data       -826.     NA
      309 multinomial  data       -826.     NA
      310 multinomial  data       -826.     NA
      311 multinomial  data       -826.     NA
      312 multinomial  data       -826.     NA
      313 multinomial  data       -826.     NA
      314 multinomial  data       -826.     NA
      315 multinomial  data       -826.     NA
      316 multinomial  data       -826.     NA
      317 multinomial  data       -826.     NA
      318 multinomial  data       -826.     NA
      319 multinomial  data       -826.     NA
      320 multinomial  data       -826.     NA
      # i 13,676 more rows

