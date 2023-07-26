#include "test.h"
 one_line_type fmod_vec[] = {
{64, 0,123,__LINE__, 0xbff33333, 0x33333333, 0xbff33333, 0x33333333, 0x40066666, 0x66666666},			/* -1.20000=f(-1.20000, v2.80000)*/
{64, 0,123,__LINE__, 0xbff30a3d, 0x70a3d70a, 0xbff30a3d, 0x70a3d70a, 0x40067ae1, 0x47ae147b},			/* -1.19000=f(-1.19000, v2.81000)*/
{64, 0,123,__LINE__, 0xbff2e147, 0xae147ae1, 0xbff2e147, 0xae147ae1, 0x40068f5c, 0x28f5c290},			/* -1.18000=f(-1.18000, v2.82000)*/
{64, 0,123,__LINE__, 0xbff2b851, 0xeb851eb8, 0xbff2b851, 0xeb851eb8, 0x4006a3d7, 0x0a3d70a4},			/* -1.17000=f(-1.17000, v2.83000)*/
{64, 0,123,__LINE__, 0xbff28f5c, 0x28f5c28f, 0xbff28f5c, 0x28f5c28f, 0x4006b851, 0xeb851eb8},			/* -1.16000=f(-1.16000, v2.84000)*/
{64, 0,123,__LINE__, 0xbff26666, 0x66666666, 0xbff26666, 0x66666666, 0x4006cccc, 0xcccccccd},			/* -1.15000=f(-1.15000, v2.85000)*/
{64, 0,123,__LINE__, 0xbff23d70, 0xa3d70a3d, 0xbff23d70, 0xa3d70a3d, 0x4006e147, 0xae147ae2},			/* -1.14000=f(-1.14000, v2.86000)*/
{64, 0,123,__LINE__, 0xbff2147a, 0xe147ae14, 0xbff2147a, 0xe147ae14, 0x4006f5c2, 0x8f5c28f6},			/* -1.13000=f(-1.13000, v2.87000)*/
{64, 0,123,__LINE__, 0xbff1eb85, 0x1eb851eb, 0xbff1eb85, 0x1eb851eb, 0x40070a3d, 0x70a3d70a},			/* -1.12000=f(-1.12000, v2.88000)*/
{64, 0,123,__LINE__, 0xbff1c28f, 0x5c28f5c2, 0xbff1c28f, 0x5c28f5c2, 0x40071eb8, 0x51eb851f},			/* -1.11000=f(-1.11000, v2.89000)*/
{64, 0,123,__LINE__, 0xbff19999, 0x99999999, 0xbff19999, 0x99999999, 0x40073333, 0x33333334},			/* -1.10000=f(-1.10000, v2.90000)*/
{64, 0,123,__LINE__, 0xbff170a3, 0xd70a3d70, 0xbff170a3, 0xd70a3d70, 0x400747ae, 0x147ae148},			/* -1.09000=f(-1.09000, v2.91000)*/
{64, 0,123,__LINE__, 0xbff147ae, 0x147ae147, 0xbff147ae, 0x147ae147, 0x40075c28, 0xf5c28f5c},			/* -1.08000=f(-1.08000, v2.92000)*/
{64, 0,123,__LINE__, 0xbff11eb8, 0x51eb851e, 0xbff11eb8, 0x51eb851e, 0x400770a3, 0xd70a3d71},			/* -1.07000=f(-1.07000, v2.93000)*/
{64, 0,123,__LINE__, 0xbff0f5c2, 0x8f5c28f5, 0xbff0f5c2, 0x8f5c28f5, 0x4007851e, 0xb851eb86},			/* -1.06000=f(-1.06000, v2.94000)*/
{64, 0,123,__LINE__, 0xbff0cccc, 0xcccccccc, 0xbff0cccc, 0xcccccccc, 0x40079999, 0x9999999a},			/* -1.05000=f(-1.05000, v2.95000)*/
{64, 0,123,__LINE__, 0xbff0a3d7, 0x0a3d70a3, 0xbff0a3d7, 0x0a3d70a3, 0x4007ae14, 0x7ae147ae},			/* -1.04000=f(-1.04000, v2.96000)*/
{64, 0,123,__LINE__, 0xbff07ae1, 0x47ae147a, 0xbff07ae1, 0x47ae147a, 0x4007c28f, 0x5c28f5c3},			/* -1.03000=f(-1.03000, v2.97000)*/
{64, 0,123,__LINE__, 0xbff051eb, 0x851eb851, 0xbff051eb, 0x851eb851, 0x4007d70a, 0x3d70a3d8},			/* -1.02000=f(-1.02000, v2.98000)*/
{64, 0,123,__LINE__, 0xbff028f5, 0xc28f5c28, 0xbff028f5, 0xc28f5c28, 0x4007eb85, 0x1eb851ec},			/* -1.01000=f(-1.01000, v2.99000)*/
{64, 0,123,__LINE__, 0xbfefffff, 0xfffffffe, 0xbfefffff, 0xfffffffe, 0x40080000, 0x00000000},			/* -0.01000=f(-0.01000, v3.00000)*/
{64, 0,123,__LINE__, 0xbfefae14, 0x7ae147ac, 0xbfefae14, 0x7ae147ac, 0x4008147a, 0xe147ae15},			/* -0.99000=f(-0.99000, v3.01000)*/
{64, 0,123,__LINE__, 0xbfef5c28, 0xf5c28f5a, 0xbfef5c28, 0xf5c28f5a, 0x400828f5, 0xc28f5c2a},			/* -0.98000=f(-0.98000, v3.02000)*/
{64, 0,123,__LINE__, 0xbfef0a3d, 0x70a3d708, 0xbfef0a3d, 0x70a3d708, 0x40083d70, 0xa3d70a3e},			/* -0.97000=f(-0.97000, v3.03000)*/
{64, 0,123,__LINE__, 0xbfeeb851, 0xeb851eb6, 0xbfeeb851, 0xeb851eb6, 0x400851eb, 0x851eb852},			/* -0.96000=f(-0.96000, v3.04000)*/
{64, 0,123,__LINE__, 0xbfee6666, 0x66666664, 0xbfee6666, 0x66666664, 0x40086666, 0x66666667},			/* -0.95000=f(-0.95000, v3.05000)*/
{64, 0,123,__LINE__, 0xbfee147a, 0xe147ae12, 0xbfee147a, 0xe147ae12, 0x40087ae1, 0x47ae147c},			/* -0.94000=f(-0.94000, v3.06000)*/
{64, 0,123,__LINE__, 0xbfedc28f, 0x5c28f5c0, 0xbfedc28f, 0x5c28f5c0, 0x40088f5c, 0x28f5c290},			/* -0.93000=f(-0.93000, v3.07000)*/
{64, 0,123,__LINE__, 0xbfed70a3, 0xd70a3d6e, 0xbfed70a3, 0xd70a3d6e, 0x4008a3d7, 0x0a3d70a4},			/* -0.92000=f(-0.92000, v3.08000)*/
{64, 0,123,__LINE__, 0xbfed1eb8, 0x51eb851c, 0xbfed1eb8, 0x51eb851c, 0x4008b851, 0xeb851eb9},			/* -0.91000=f(-0.91000, v3.09000)*/
{64, 0,123,__LINE__, 0xbfeccccc, 0xccccccca, 0xbfeccccc, 0xccccccca, 0x4008cccc, 0xccccccce},			/* -0.90000=f(-0.90000, v3.10000)*/
{64, 0,123,__LINE__, 0xbfec7ae1, 0x47ae1478, 0xbfec7ae1, 0x47ae1478, 0x4008e147, 0xae147ae2},			/* -0.89000=f(-0.89000, v3.11000)*/
{64, 0,123,__LINE__, 0xbfec28f5, 0xc28f5c26, 0xbfec28f5, 0xc28f5c26, 0x4008f5c2, 0x8f5c28f6},			/* -0.88000=f(-0.88000, v3.12000)*/
{64, 0,123,__LINE__, 0xbfebd70a, 0x3d70a3d4, 0xbfebd70a, 0x3d70a3d4, 0x40090a3d, 0x70a3d70b},			/* -0.87000=f(-0.87000, v3.13000)*/
{64, 0,123,__LINE__, 0xbfeb851e, 0xb851eb82, 0xbfeb851e, 0xb851eb82, 0x40091eb8, 0x51eb8520},			/* -0.86000=f(-0.86000, v3.14000)*/
{64, 0,123,__LINE__, 0xbfeb3333, 0x33333330, 0xbfeb3333, 0x33333330, 0x40093333, 0x33333334},			/* -0.85000=f(-0.85000, v3.15000)*/
{64, 0,123,__LINE__, 0xbfeae147, 0xae147ade, 0xbfeae147, 0xae147ade, 0x400947ae, 0x147ae148},			/* -0.84000=f(-0.84000, v3.16000)*/
{64, 0,123,__LINE__, 0xbfea8f5c, 0x28f5c28c, 0xbfea8f5c, 0x28f5c28c, 0x40095c28, 0xf5c28f5d},			/* -0.83000=f(-0.83000, v3.17000)*/
{64, 0,123,__LINE__, 0xbfea3d70, 0xa3d70a3a, 0xbfea3d70, 0xa3d70a3a, 0x400970a3, 0xd70a3d72},			/* -0.82000=f(-0.82000, v3.18000)*/
{64, 0,123,__LINE__, 0xbfe9eb85, 0x1eb851e8, 0xbfe9eb85, 0x1eb851e8, 0x4009851e, 0xb851eb86},			/* -0.81000=f(-0.81000, v3.19000)*/
{64, 0,123,__LINE__, 0xbfe99999, 0x99999996, 0xbfe99999, 0x99999996, 0x40099999, 0x9999999a},			/* -0.80000=f(-0.80000, v3.20000)*/
{64, 0,123,__LINE__, 0xbfe947ae, 0x147ae144, 0xbfe947ae, 0x147ae144, 0x4009ae14, 0x7ae147af},			/* -0.79000=f(-0.79000, v3.21000)*/
{64, 0,123,__LINE__, 0xbfe8f5c2, 0x8f5c28f2, 0xbfe8f5c2, 0x8f5c28f2, 0x4009c28f, 0x5c28f5c4},			/* -0.78000=f(-0.78000, v3.22000)*/
{64, 0,123,__LINE__, 0xbfe8a3d7, 0x0a3d70a0, 0xbfe8a3d7, 0x0a3d70a0, 0x4009d70a, 0x3d70a3d8},			/* -0.77000=f(-0.77000, v3.23000)*/
{64, 0,123,__LINE__, 0xbfe851eb, 0x851eb84e, 0xbfe851eb, 0x851eb84e, 0x4009eb85, 0x1eb851ec},			/* -0.76000=f(-0.76000, v3.24000)*/
{64, 0,123,__LINE__, 0xbfe7ffff, 0xfffffffc, 0xbfe7ffff, 0xfffffffc, 0x400a0000, 0x00000001},			/* -0.75000=f(-0.75000, v3.25000)*/
{64, 0,123,__LINE__, 0xbfe7ae14, 0x7ae147aa, 0xbfe7ae14, 0x7ae147aa, 0x400a147a, 0xe147ae16},			/* -0.74000=f(-0.74000, v3.26000)*/
{64, 0,123,__LINE__, 0xbfe75c28, 0xf5c28f58, 0xbfe75c28, 0xf5c28f58, 0x400a28f5, 0xc28f5c2a},			/* -0.73000=f(-0.73000, v3.27000)*/
{64, 0,123,__LINE__, 0xbfe70a3d, 0x70a3d706, 0xbfe70a3d, 0x70a3d706, 0x400a3d70, 0xa3d70a3e},			/* -0.72000=f(-0.72000, v3.28000)*/
{64, 0,123,__LINE__, 0xbfe6b851, 0xeb851eb4, 0xbfe6b851, 0xeb851eb4, 0x400a51eb, 0x851eb853},			/* -0.71000=f(-0.71000, v3.29000)*/
{64, 0,123,__LINE__, 0xbfe66666, 0x66666662, 0xbfe66666, 0x66666662, 0x400a6666, 0x66666668},			/* -0.70000=f(-0.70000, v3.30000)*/
{64, 0,123,__LINE__, 0xbfe6147a, 0xe147ae10, 0xbfe6147a, 0xe147ae10, 0x400a7ae1, 0x47ae147c},			/* -0.69000=f(-0.69000, v3.31000)*/
{64, 0,123,__LINE__, 0xbfe5c28f, 0x5c28f5be, 0xbfe5c28f, 0x5c28f5be, 0x400a8f5c, 0x28f5c290},			/* -0.68000=f(-0.68000, v3.32000)*/
{64, 0,123,__LINE__, 0xbfe570a3, 0xd70a3d6c, 0xbfe570a3, 0xd70a3d6c, 0x400aa3d7, 0x0a3d70a5},			/* -0.67000=f(-0.67000, v3.33000)*/
{64, 0,123,__LINE__, 0xbfe51eb8, 0x51eb851a, 0xbfe51eb8, 0x51eb851a, 0x400ab851, 0xeb851eba},			/* -0.66000=f(-0.66000, v3.34000)*/
{64, 0,123,__LINE__, 0xbfe4cccc, 0xccccccc8, 0xbfe4cccc, 0xccccccc8, 0x400acccc, 0xccccccce},			/* -0.65000=f(-0.65000, v3.35000)*/
{64, 0,123,__LINE__, 0xbfe47ae1, 0x47ae1476, 0xbfe47ae1, 0x47ae1476, 0x400ae147, 0xae147ae2},			/* -0.64000=f(-0.64000, v3.36000)*/
{64, 0,123,__LINE__, 0xbfe428f5, 0xc28f5c24, 0xbfe428f5, 0xc28f5c24, 0x400af5c2, 0x8f5c28f7},			/* -0.63000=f(-0.63000, v3.37000)*/
{64, 0,123,__LINE__, 0xbfe3d70a, 0x3d70a3d2, 0xbfe3d70a, 0x3d70a3d2, 0x400b0a3d, 0x70a3d70c},			/* -0.62000=f(-0.62000, v3.38000)*/
{64, 0,123,__LINE__, 0xbfe3851e, 0xb851eb80, 0xbfe3851e, 0xb851eb80, 0x400b1eb8, 0x51eb8520},			/* -0.61000=f(-0.61000, v3.39000)*/
{64, 0,123,__LINE__, 0xbfe33333, 0x3333332e, 0xbfe33333, 0x3333332e, 0x400b3333, 0x33333334},			/* -0.60000=f(-0.60000, v3.40000)*/
{64, 0,123,__LINE__, 0xbfe2e147, 0xae147adc, 0xbfe2e147, 0xae147adc, 0x400b47ae, 0x147ae149},			/* -0.59000=f(-0.59000, v3.41000)*/
{64, 0,123,__LINE__, 0xbfe28f5c, 0x28f5c28a, 0xbfe28f5c, 0x28f5c28a, 0x400b5c28, 0xf5c28f5e},			/* -0.58000=f(-0.58000, v3.42000)*/
{64, 0,123,__LINE__, 0xbfe23d70, 0xa3d70a38, 0xbfe23d70, 0xa3d70a38, 0x400b70a3, 0xd70a3d72},			/* -0.57000=f(-0.57000, v3.43000)*/
{64, 0,123,__LINE__, 0xbfe1eb85, 0x1eb851e6, 0xbfe1eb85, 0x1eb851e6, 0x400b851e, 0xb851eb86},			/* -0.56000=f(-0.56000, v3.44000)*/
{64, 0,123,__LINE__, 0xbfe19999, 0x99999994, 0xbfe19999, 0x99999994, 0x400b9999, 0x9999999b},			/* -0.55000=f(-0.55000, v3.45000)*/
{64, 0,123,__LINE__, 0xbfe147ae, 0x147ae142, 0xbfe147ae, 0x147ae142, 0x400bae14, 0x7ae147b0},			/* -0.54000=f(-0.54000, v3.46000)*/
{64, 0,123,__LINE__, 0xbfe0f5c2, 0x8f5c28f0, 0xbfe0f5c2, 0x8f5c28f0, 0x400bc28f, 0x5c28f5c4},			/* -0.53000=f(-0.53000, v3.47000)*/
{64, 0,123,__LINE__, 0xbfe0a3d7, 0x0a3d709e, 0xbfe0a3d7, 0x0a3d709e, 0x400bd70a, 0x3d70a3d8},			/* -0.52000=f(-0.52000, v3.48000)*/
{64, 0,123,__LINE__, 0xbfe051eb, 0x851eb84c, 0xbfe051eb, 0x851eb84c, 0x400beb85, 0x1eb851ed},			/* -0.51000=f(-0.51000, v3.49000)*/
{64, 0,123,__LINE__, 0xbfdfffff, 0xfffffff4, 0xbfdfffff, 0xfffffff4, 0x400c0000, 0x00000002},			/* -0.50000=f(-0.50000, v3.50000)*/
{64, 0,123,__LINE__, 0xbfdf5c28, 0xf5c28f50, 0xbfdf5c28, 0xf5c28f50, 0x400c147a, 0xe147ae16},			/* -0.49000=f(-0.49000, v3.51000)*/
{64, 0,123,__LINE__, 0xbfdeb851, 0xeb851eac, 0xbfdeb851, 0xeb851eac, 0x400c28f5, 0xc28f5c2a},			/* -0.48000=f(-0.48000, v3.52000)*/
{64, 0,123,__LINE__, 0xbfde147a, 0xe147ae08, 0xbfde147a, 0xe147ae08, 0x400c3d70, 0xa3d70a3f},			/* -0.47000=f(-0.47000, v3.53000)*/
{64, 0,123,__LINE__, 0xbfdd70a3, 0xd70a3d64, 0xbfdd70a3, 0xd70a3d64, 0x400c51eb, 0x851eb854},			/* -0.46000=f(-0.46000, v3.54000)*/
{64, 0,123,__LINE__, 0xbfdccccc, 0xccccccc0, 0xbfdccccc, 0xccccccc0, 0x400c6666, 0x66666668},			/* -0.45000=f(-0.45000, v3.55000)*/
{64, 0,123,__LINE__, 0xbfdc28f5, 0xc28f5c1c, 0xbfdc28f5, 0xc28f5c1c, 0x400c7ae1, 0x47ae147c},			/* -0.44000=f(-0.44000, v3.56000)*/
{64, 0,123,__LINE__, 0xbfdb851e, 0xb851eb78, 0xbfdb851e, 0xb851eb78, 0x400c8f5c, 0x28f5c291},			/* -0.43000=f(-0.43000, v3.57000)*/
{64, 0,123,__LINE__, 0xbfdae147, 0xae147ad4, 0xbfdae147, 0xae147ad4, 0x400ca3d7, 0x0a3d70a6},			/* -0.42000=f(-0.42000, v3.58000)*/
{64, 0,123,__LINE__, 0xbfda3d70, 0xa3d70a30, 0xbfda3d70, 0xa3d70a30, 0x400cb851, 0xeb851eba},			/* -0.41000=f(-0.41000, v3.59000)*/
{64, 0,123,__LINE__, 0xbfd99999, 0x9999998c, 0xbfd99999, 0x9999998c, 0x400ccccc, 0xccccccce},			/* -0.40000=f(-0.40000, v3.60000)*/
{64, 0,123,__LINE__, 0xbfd8f5c2, 0x8f5c28e8, 0xbfd8f5c2, 0x8f5c28e8, 0x400ce147, 0xae147ae3},			/* -0.39000=f(-0.39000, v3.61000)*/
{64, 0,123,__LINE__, 0xbfd851eb, 0x851eb844, 0xbfd851eb, 0x851eb844, 0x400cf5c2, 0x8f5c28f8},			/* -0.38000=f(-0.38000, v3.62000)*/
{64, 0,123,__LINE__, 0xbfd7ae14, 0x7ae147a0, 0xbfd7ae14, 0x7ae147a0, 0x400d0a3d, 0x70a3d70c},			/* -0.37000=f(-0.37000, v3.63000)*/
{64, 0,123,__LINE__, 0xbfd70a3d, 0x70a3d6fc, 0xbfd70a3d, 0x70a3d6fc, 0x400d1eb8, 0x51eb8520},			/* -0.36000=f(-0.36000, v3.64000)*/
{64, 0,123,__LINE__, 0xbfd66666, 0x66666658, 0xbfd66666, 0x66666658, 0x400d3333, 0x33333335},			/* -0.35000=f(-0.35000, v3.65000)*/
{64, 0,123,__LINE__, 0xbfd5c28f, 0x5c28f5b4, 0xbfd5c28f, 0x5c28f5b4, 0x400d47ae, 0x147ae14a},			/* -0.34000=f(-0.34000, v3.66000)*/
{64, 0,123,__LINE__, 0xbfd51eb8, 0x51eb8510, 0xbfd51eb8, 0x51eb8510, 0x400d5c28, 0xf5c28f5e},			/* -0.33000=f(-0.33000, v3.67000)*/
{64, 0,123,__LINE__, 0xbfd47ae1, 0x47ae146c, 0xbfd47ae1, 0x47ae146c, 0x400d70a3, 0xd70a3d72},			/* -0.32000=f(-0.32000, v3.68000)*/
{64, 0,123,__LINE__, 0xbfd3d70a, 0x3d70a3c8, 0xbfd3d70a, 0x3d70a3c8, 0x400d851e, 0xb851eb87},			/* -0.31000=f(-0.31000, v3.69000)*/
{64, 0,123,__LINE__, 0xbfd33333, 0x33333324, 0xbfd33333, 0x33333324, 0x400d9999, 0x9999999c},			/* -0.30000=f(-0.30000, v3.70000)*/
{64, 0,123,__LINE__, 0xbfd28f5c, 0x28f5c280, 0xbfd28f5c, 0x28f5c280, 0x400dae14, 0x7ae147b0},			/* -0.29000=f(-0.29000, v3.71000)*/
{64, 0,123,__LINE__, 0xbfd1eb85, 0x1eb851dc, 0xbfd1eb85, 0x1eb851dc, 0x400dc28f, 0x5c28f5c4},			/* -0.28000=f(-0.28000, v3.72000)*/
{64, 0,123,__LINE__, 0xbfd147ae, 0x147ae138, 0xbfd147ae, 0x147ae138, 0x400dd70a, 0x3d70a3d9},			/* -0.27000=f(-0.27000, v3.73000)*/
{64, 0,123,__LINE__, 0xbfd0a3d7, 0x0a3d7094, 0xbfd0a3d7, 0x0a3d7094, 0x400deb85, 0x1eb851ee},			/* -0.26000=f(-0.26000, v3.74000)*/
{64, 0,123,__LINE__, 0xbfcfffff, 0xffffffe0, 0xbfcfffff, 0xffffffe0, 0x400e0000, 0x00000002},			/* -0.25000=f(-0.25000, v3.75000)*/
{64, 0,123,__LINE__, 0xbfceb851, 0xeb851e98, 0xbfceb851, 0xeb851e98, 0x400e147a, 0xe147ae16},			/* -0.24000=f(-0.24000, v3.76000)*/
{64, 0,123,__LINE__, 0xbfcd70a3, 0xd70a3d50, 0xbfcd70a3, 0xd70a3d50, 0x400e28f5, 0xc28f5c2b},			/* -0.23000=f(-0.23000, v3.77000)*/
{64, 0,123,__LINE__, 0xbfcc28f5, 0xc28f5c08, 0xbfcc28f5, 0xc28f5c08, 0x400e3d70, 0xa3d70a40},			/* -0.22000=f(-0.22000, v3.78000)*/
{64, 0,123,__LINE__, 0xbfcae147, 0xae147ac0, 0xbfcae147, 0xae147ac0, 0x400e51eb, 0x851eb854},			/* -0.21000=f(-0.21000, v3.79000)*/
{64, 0,123,__LINE__, 0xbfc99999, 0x99999978, 0xbfc99999, 0x99999978, 0x400e6666, 0x66666668},			/* -0.20000=f(-0.20000, v3.80000)*/
{64, 0,123,__LINE__, 0xbfc851eb, 0x851eb830, 0xbfc851eb, 0x851eb830, 0x400e7ae1, 0x47ae147d},			/* -0.19000=f(-0.19000, v3.81000)*/
{64, 0,123,__LINE__, 0xbfc70a3d, 0x70a3d6e8, 0xbfc70a3d, 0x70a3d6e8, 0x400e8f5c, 0x28f5c292},			/* -0.18000=f(-0.18000, v3.82000)*/
{64, 0,123,__LINE__, 0xbfc5c28f, 0x5c28f5a0, 0xbfc5c28f, 0x5c28f5a0, 0x400ea3d7, 0x0a3d70a6},			/* -0.17000=f(-0.17000, v3.83000)*/
{64, 0,123,__LINE__, 0xbfc47ae1, 0x47ae1458, 0xbfc47ae1, 0x47ae1458, 0x400eb851, 0xeb851eba},			/* -0.16000=f(-0.16000, v3.84000)*/
{64, 0,123,__LINE__, 0xbfc33333, 0x33333310, 0xbfc33333, 0x33333310, 0x400ecccc, 0xcccccccf},			/* -0.15000=f(-0.15000, v3.85000)*/
{64, 0,123,__LINE__, 0xbfc1eb85, 0x1eb851c8, 0xbfc1eb85, 0x1eb851c8, 0x400ee147, 0xae147ae4},			/* -0.14000=f(-0.14000, v3.86000)*/
{64, 0,123,__LINE__, 0xbfc0a3d7, 0x0a3d7080, 0xbfc0a3d7, 0x0a3d7080, 0x400ef5c2, 0x8f5c28f8},			/* -0.13000=f(-0.13000, v3.87000)*/
{64, 0,123,__LINE__, 0xbfbeb851, 0xeb851e71, 0xbfbeb851, 0xeb851e71, 0x400f0a3d, 0x70a3d70c},			/* -0.12000=f(-0.12000, v3.88000)*/
{64, 0,123,__LINE__, 0xbfbc28f5, 0xc28f5be2, 0xbfbc28f5, 0xc28f5be2, 0x400f1eb8, 0x51eb8521},			/* -0.11000=f(-0.11000, v3.89000)*/
{64, 0,123,__LINE__, 0xbfb99999, 0x99999953, 0xbfb99999, 0x99999953, 0x400f3333, 0x33333335},			/* -0.00100=f(-0.00100, v3.90000)*/
{64, 0,123,__LINE__, 0xbfb70a3d, 0x70a3d6c4, 0xbfb70a3d, 0x70a3d6c4, 0x400f47ae, 0x147ae14a},			/* -0.09000=f(-0.09000, v3.91000)*/
{64, 0,123,__LINE__, 0xbfb47ae1, 0x47ae1435, 0xbfb47ae1, 0x47ae1435, 0x400f5c28, 0xf5c28f5e},			/* -0.08000=f(-0.08000, v3.92000)*/
{64, 0,123,__LINE__, 0xbfb1eb85, 0x1eb851a6, 0xbfb1eb85, 0x1eb851a6, 0x400f70a3, 0xd70a3d73},			/* -0.07000=f(-0.07000, v3.93000)*/
{64, 0,123,__LINE__, 0xbfaeb851, 0xeb851e2d, 0xbfaeb851, 0xeb851e2d, 0x400f851e, 0xb851eb87},			/* -0.06000=f(-0.06000, v3.94000)*/
{64, 0,123,__LINE__, 0xbfa99999, 0x9999990e, 0xbfa99999, 0x9999990e, 0x400f9999, 0x9999999c},			/* -0.05000=f(-0.05000, v3.95000)*/
{64, 0,123,__LINE__, 0xbfa47ae1, 0x47ae13ef, 0xbfa47ae1, 0x47ae13ef, 0x400fae14, 0x7ae147b0},			/* -0.04000=f(-0.04000, v3.96000)*/
{64, 0,123,__LINE__, 0xbf9eb851, 0xeb851da0, 0xbf9eb851, 0xeb851da0, 0x400fc28f, 0x5c28f5c5},			/* -0.03000=f(-0.03000, v3.97000)*/
{64, 0,123,__LINE__, 0xbf947ae1, 0x47ae1362, 0xbf947ae1, 0x47ae1362, 0x400fd70a, 0x3d70a3d9},			/* -0.02000=f(-0.02000, v3.98000)*/
{64, 0,123,__LINE__, 0xbf847ae1, 0x47ae1249, 0xbf847ae1, 0x47ae1249, 0x400feb85, 0x1eb851ee},			/* -0.00010=f(-0.00010, v3.99000)*/
{64, 0,123,__LINE__, 0x3cd19000, 0x00000000, 0x3cd19000, 0x00000000, 0x40100000, 0x00000001},			/* 9.74915e-16=f(9.74915e-16, v4.00000)*/
{64, 0,123,__LINE__, 0x3f847ae1, 0x47ae16ad, 0x3f847ae1, 0x47ae16ad, 0x40100a3d, 0x70a3d70b},			/* 0.01000=f(0.01000, v4.01000)*/
{64, 0,123,__LINE__, 0x3f947ae1, 0x47ae1594, 0x3f947ae1, 0x47ae1594, 0x4010147a, 0xe147ae16},			/* 0.02000=f(0.02000, v4.02000)*/
{64, 0,123,__LINE__, 0x3f9eb851, 0xeb851fd2, 0x3f9eb851, 0xeb851fd2, 0x40101eb8, 0x51eb8520},			/* 0.03000=f(0.03000, v4.03000)*/
{64, 0,123,__LINE__, 0x3fa47ae1, 0x47ae1508, 0x3fa47ae1, 0x47ae1508, 0x401028f5, 0xc28f5c2a},			/* 0.04000=f(0.04000, v4.04000)*/
{64, 0,123,__LINE__, 0x3fa99999, 0x99999a27, 0x3fa99999, 0x99999a27, 0x40103333, 0x33333334},			/* 0.05000=f(0.05000, v4.05000)*/
{64, 0,123,__LINE__, 0x3faeb851, 0xeb851f46, 0x3faeb851, 0xeb851f46, 0x40103d70, 0xa3d70a3f},			/* 0.06000=f(0.06000, v4.06000)*/
{64, 0,123,__LINE__, 0x3fb1eb85, 0x1eb85232, 0x3fb1eb85, 0x1eb85232, 0x401047ae, 0x147ae149},			/* 0.07000=f(0.07000, v4.07000)*/
{64, 0,123,__LINE__, 0x3fb47ae1, 0x47ae14c1, 0x3fb47ae1, 0x47ae14c1, 0x401051eb, 0x851eb853},			/* 0.08000=f(0.08000, v4.08000)*/
{64, 0,123,__LINE__, 0x3fb70a3d, 0x70a3d750, 0x3fb70a3d, 0x70a3d750, 0x40105c28, 0xf5c28f5d},			/* 0.09000=f(0.09000, v4.09000)*/
{64, 0,123,__LINE__, 0x3fb99999, 0x999999df, 0x3fb99999, 0x999999df, 0x40106666, 0x66666667},			/* 0.10000=f(0.10000, v4.10000)*/
{64, 0,123,__LINE__, 0x3fbc28f5, 0xc28f5c6e, 0x3fbc28f5, 0xc28f5c6e, 0x401070a3, 0xd70a3d72},			/* 0.11000=f(0.11000, v4.11000)*/
{64, 0,123,__LINE__, 0x3fbeb851, 0xeb851efd, 0x3fbeb851, 0xeb851efd, 0x40107ae1, 0x47ae147c},			/* 0.12000=f(0.12000, v4.12000)*/
{64, 0,123,__LINE__, 0x3fc0a3d7, 0x0a3d70c6, 0x3fc0a3d7, 0x0a3d70c6, 0x4010851e, 0xb851eb86},			/* 0.13000=f(0.13000, v4.13000)*/
{64, 0,123,__LINE__, 0x3fc1eb85, 0x1eb8520e, 0x3fc1eb85, 0x1eb8520e, 0x40108f5c, 0x28f5c290},			/* 0.14000=f(0.14000, v4.14000)*/
{64, 0,123,__LINE__, 0x3fc33333, 0x33333356, 0x3fc33333, 0x33333356, 0x40109999, 0x9999999b},			/* 0.15000=f(0.15000, v4.15000)*/
{64, 0,123,__LINE__, 0x3fc47ae1, 0x47ae149e, 0x3fc47ae1, 0x47ae149e, 0x4010a3d7, 0x0a3d70a5},			/* 0.16000=f(0.16000, v4.16000)*/
{64, 0,123,__LINE__, 0x3fc5c28f, 0x5c28f5e6, 0x3fc5c28f, 0x5c28f5e6, 0x4010ae14, 0x7ae147af},			/* 0.17000=f(0.17000, v4.17000)*/
{64, 0,123,__LINE__, 0x3fc70a3d, 0x70a3d72e, 0x3fc70a3d, 0x70a3d72e, 0x4010b851, 0xeb851eb9},			/* 0.18000=f(0.18000, v4.18000)*/
{64, 0,123,__LINE__, 0x3fc851eb, 0x851eb876, 0x3fc851eb, 0x851eb876, 0x4010c28f, 0x5c28f5c4},			/* 0.19000=f(0.19000, v4.19000)*/
{64, 0,123,__LINE__, 0x3fc99999, 0x999999be, 0x3fc99999, 0x999999be, 0x4010cccc, 0xccccccce},			/* 0.20000=f(0.20000, v4.20000)*/
{64, 0,123,__LINE__, 0x3fcae147, 0xae147b06, 0x3fcae147, 0xae147b06, 0x4010d70a, 0x3d70a3d8},			/* 0.21000=f(0.21000, v4.21000)*/
{64, 0,123,__LINE__, 0x3fcc28f5, 0xc28f5c4e, 0x3fcc28f5, 0xc28f5c4e, 0x4010e147, 0xae147ae2},			/* 0.22000=f(0.22000, v4.22000)*/
{64, 0,123,__LINE__, 0x3fcd70a3, 0xd70a3d96, 0x3fcd70a3, 0xd70a3d96, 0x4010eb85, 0x1eb851ed},			/* 0.23000=f(0.23000, v4.23000)*/
{64, 0,123,__LINE__, 0x3fceb851, 0xeb851ede, 0x3fceb851, 0xeb851ede, 0x4010f5c2, 0x8f5c28f7},			/* 0.24000=f(0.24000, v4.24000)*/
{64, 0,123,__LINE__, 0x3fd00000, 0x00000013, 0x3fd00000, 0x00000013, 0x40110000, 0x00000001},			/* 0.25000=f(0.25000, v4.25000)*/
{64, 0,123,__LINE__, 0x3fd0a3d7, 0x0a3d70b7, 0x3fd0a3d7, 0x0a3d70b7, 0x40110a3d, 0x70a3d70b},			/* 0.26000=f(0.26000, v4.26000)*/
{64, 0,123,__LINE__, 0x3fd147ae, 0x147ae15b, 0x3fd147ae, 0x147ae15b, 0x4011147a, 0xe147ae16},			/* 0.27000=f(0.27000, v4.27000)*/
{64, 0,123,__LINE__, 0x3fd1eb85, 0x1eb851ff, 0x3fd1eb85, 0x1eb851ff, 0x40111eb8, 0x51eb8520},			/* 0.28000=f(0.28000, v4.28000)*/
{64, 0,123,__LINE__, 0x3fd28f5c, 0x28f5c2a3, 0x3fd28f5c, 0x28f5c2a3, 0x401128f5, 0xc28f5c2a},			/* 0.29000=f(0.29000, v4.29000)*/
{64, 0,123,__LINE__, 0x3fd33333, 0x33333347, 0x3fd33333, 0x33333347, 0x40113333, 0x33333334},			/* 0.30000=f(0.30000, v4.30000)*/
{64, 0,123,__LINE__, 0x3fd3d70a, 0x3d70a3eb, 0x3fd3d70a, 0x3d70a3eb, 0x40113d70, 0xa3d70a3f},			/* 0.31000=f(0.31000, v4.31000)*/
{64, 0,123,__LINE__, 0x3fd47ae1, 0x47ae148f, 0x3fd47ae1, 0x47ae148f, 0x401147ae, 0x147ae149},			/* 0.32000=f(0.32000, v4.32000)*/
{64, 0,123,__LINE__, 0x3fd51eb8, 0x51eb8533, 0x3fd51eb8, 0x51eb8533, 0x401151eb, 0x851eb853},			/* 0.33000=f(0.33000, v4.33000)*/
{64, 0,123,__LINE__, 0x3fd5c28f, 0x5c28f5d7, 0x3fd5c28f, 0x5c28f5d7, 0x40115c28, 0xf5c28f5d},			/* 0.34000=f(0.34000, v4.34000)*/
{64, 0,123,__LINE__, 0x3fd66666, 0x6666667b, 0x3fd66666, 0x6666667b, 0x40116666, 0x66666668},			/* 0.35000=f(0.35000, v4.35000)*/
{64, 0,123,__LINE__, 0x3fd70a3d, 0x70a3d71f, 0x3fd70a3d, 0x70a3d71f, 0x401170a3, 0xd70a3d72},			/* 0.36000=f(0.36000, v4.36000)*/
{64, 0,123,__LINE__, 0x3fd7ae14, 0x7ae147c3, 0x3fd7ae14, 0x7ae147c3, 0x40117ae1, 0x47ae147c},			/* 0.37000=f(0.37000, v4.37000)*/
{64, 0,123,__LINE__, 0x3fd851eb, 0x851eb867, 0x3fd851eb, 0x851eb867, 0x4011851e, 0xb851eb86},			/* 0.38000=f(0.38000, v4.38000)*/
{64, 0,123,__LINE__, 0x3fd8f5c2, 0x8f5c290b, 0x3fd8f5c2, 0x8f5c290b, 0x40118f5c, 0x28f5c291},			/* 0.39000=f(0.39000, v4.39000)*/
{64, 0,123,__LINE__, 0x3fd99999, 0x999999af, 0x3fd99999, 0x999999af, 0x40119999, 0x9999999b},			/* 0.40000=f(0.40000, v4.40000)*/
{64, 0,123,__LINE__, 0x3fda3d70, 0xa3d70a53, 0x3fda3d70, 0xa3d70a53, 0x4011a3d7, 0x0a3d70a5},			/* 0.41000=f(0.41000, v4.41000)*/
{64, 0,123,__LINE__, 0x3fdae147, 0xae147af7, 0x3fdae147, 0xae147af7, 0x4011ae14, 0x7ae147af},			/* 0.42000=f(0.42000, v4.42000)*/
{64, 0,123,__LINE__, 0x3fdb851e, 0xb851eb9b, 0x3fdb851e, 0xb851eb9b, 0x4011b851, 0xeb851eba},			/* 0.43000=f(0.43000, v4.43000)*/
{64, 0,123,__LINE__, 0x3fdc28f5, 0xc28f5c3f, 0x3fdc28f5, 0xc28f5c3f, 0x4011c28f, 0x5c28f5c4},			/* 0.44000=f(0.44000, v4.44000)*/
{64, 0,123,__LINE__, 0x3fdccccc, 0xcccccce3, 0x3fdccccc, 0xcccccce3, 0x4011cccc, 0xccccccce},			/* 0.45000=f(0.45000, v4.45000)*/
{64, 0,123,__LINE__, 0x3fdd70a3, 0xd70a3d87, 0x3fdd70a3, 0xd70a3d87, 0x4011d70a, 0x3d70a3d8},			/* 0.46000=f(0.46000, v4.46000)*/
{64, 0,123,__LINE__, 0x3fde147a, 0xe147ae2b, 0x3fde147a, 0xe147ae2b, 0x4011e147, 0xae147ae3},			/* 0.47000=f(0.47000, v4.47000)*/
{64, 0,123,__LINE__, 0x3fdeb851, 0xeb851ecf, 0x3fdeb851, 0xeb851ecf, 0x4011eb85, 0x1eb851ed},			/* 0.48000=f(0.48000, v4.48000)*/
{64, 0,123,__LINE__, 0x3fdf5c28, 0xf5c28f73, 0x3fdf5c28, 0xf5c28f73, 0x4011f5c2, 0x8f5c28f7},			/* 0.49000=f(0.49000, v4.49000)*/
{64, 0,123,__LINE__, 0x3fe00000, 0x0000000b, 0x3fe00000, 0x0000000b, 0x40120000, 0x00000001},			/* 0.50000=f(0.50000, v4.50000)*/
{64, 0,123,__LINE__, 0x3fe051eb, 0x851eb85d, 0x3fe051eb, 0x851eb85d, 0x40120a3d, 0x70a3d70c},			/* 0.51000=f(0.51000, v4.51000)*/
{64, 0,123,__LINE__, 0x3fe0a3d7, 0x0a3d70af, 0x3fe0a3d7, 0x0a3d70af, 0x4012147a, 0xe147ae16},			/* 0.52000=f(0.52000, v4.52000)*/
{64, 0,123,__LINE__, 0x3fe0f5c2, 0x8f5c2901, 0x3fe0f5c2, 0x8f5c2901, 0x40121eb8, 0x51eb8520},			/* 0.53000=f(0.53000, v4.53000)*/
{64, 0,123,__LINE__, 0x3fe147ae, 0x147ae153, 0x3fe147ae, 0x147ae153, 0x401228f5, 0xc28f5c2a},			/* 0.54000=f(0.54000, v4.54000)*/
{64, 0,123,__LINE__, 0x3fe19999, 0x999999a5, 0x3fe19999, 0x999999a5, 0x40123333, 0x33333335},			/* 0.55000=f(0.55000, v4.55000)*/
{64, 0,123,__LINE__, 0x3fe1eb85, 0x1eb851f7, 0x3fe1eb85, 0x1eb851f7, 0x40123d70, 0xa3d70a3f},			/* 0.56000=f(0.56000, v4.56000)*/
{64, 0,123,__LINE__, 0x3fe23d70, 0xa3d70a49, 0x3fe23d70, 0xa3d70a49, 0x401247ae, 0x147ae149},			/* 0.57000=f(0.57000, v4.57000)*/
{64, 0,123,__LINE__, 0x3fe28f5c, 0x28f5c29b, 0x3fe28f5c, 0x28f5c29b, 0x401251eb, 0x851eb853},			/* 0.58000=f(0.58000, v4.58000)*/
{64, 0,123,__LINE__, 0x3fe2e147, 0xae147aed, 0x3fe2e147, 0xae147aed, 0x40125c28, 0xf5c28f5e},			/* 0.59000=f(0.59000, v4.59000)*/
{64, 0,123,__LINE__, 0x3fe33333, 0x3333333f, 0x3fe33333, 0x3333333f, 0x40126666, 0x66666668},			/* 0.60000=f(0.60000, v4.60000)*/
{64, 0,123,__LINE__, 0x3fe3851e, 0xb851eb91, 0x3fe3851e, 0xb851eb91, 0x401270a3, 0xd70a3d72},			/* 0.61000=f(0.61000, v4.61000)*/
{64, 0,123,__LINE__, 0x3fe3d70a, 0x3d70a3e3, 0x3fe3d70a, 0x3d70a3e3, 0x40127ae1, 0x47ae147c},			/* 0.62000=f(0.62000, v4.62000)*/
{64, 0,123,__LINE__, 0x3fe428f5, 0xc28f5c35, 0x3fe428f5, 0xc28f5c35, 0x4012851e, 0xb851eb87},			/* 0.63000=f(0.63000, v4.63000)*/
{64, 0,123,__LINE__, 0x3fe47ae1, 0x47ae1487, 0x3fe47ae1, 0x47ae1487, 0x40128f5c, 0x28f5c291},			/* 0.64000=f(0.64000, v4.64000)*/
{64, 0,123,__LINE__, 0x3fe4cccc, 0xccccccd9, 0x3fe4cccc, 0xccccccd9, 0x40129999, 0x9999999b},			/* 0.65000=f(0.65000, v4.65000)*/
{64, 0,123,__LINE__, 0x3fe51eb8, 0x51eb852b, 0x3fe51eb8, 0x51eb852b, 0x4012a3d7, 0x0a3d70a5},			/* 0.66000=f(0.66000, v4.66000)*/
{64, 0,123,__LINE__, 0x3fe570a3, 0xd70a3d7d, 0x3fe570a3, 0xd70a3d7d, 0x4012ae14, 0x7ae147b0},			/* 0.67000=f(0.67000, v4.67000)*/
{64, 0,123,__LINE__, 0x3fe5c28f, 0x5c28f5cf, 0x3fe5c28f, 0x5c28f5cf, 0x4012b851, 0xeb851eba},			/* 0.68000=f(0.68000, v4.68000)*/
{64, 0,123,__LINE__, 0x3fe6147a, 0xe147ae21, 0x3fe6147a, 0xe147ae21, 0x4012c28f, 0x5c28f5c4},			/* 0.69000=f(0.69000, v4.69000)*/
{64, 0,123,__LINE__, 0x3fe66666, 0x66666673, 0x3fe66666, 0x66666673, 0x4012cccc, 0xccccccce},			/* 0.70000=f(0.70000, v4.70000)*/
{64, 0,123,__LINE__, 0x3fe6b851, 0xeb851ec5, 0x3fe6b851, 0xeb851ec5, 0x4012d70a, 0x3d70a3d9},			/* 0.71000=f(0.71000, v4.71000)*/
{64, 0,123,__LINE__, 0x3fe70a3d, 0x70a3d717, 0x3fe70a3d, 0x70a3d717, 0x4012e147, 0xae147ae3},			/* 0.72000=f(0.72000, v4.72000)*/
{64, 0,123,__LINE__, 0x3fe75c28, 0xf5c28f69, 0x3fe75c28, 0xf5c28f69, 0x4012eb85, 0x1eb851ed},			/* 0.73000=f(0.73000, v4.73000)*/
{64, 0,123,__LINE__, 0x3fe7ae14, 0x7ae147bb, 0x3fe7ae14, 0x7ae147bb, 0x4012f5c2, 0x8f5c28f7},			/* 0.74000=f(0.74000, v4.74000)*/
{64, 0,123,__LINE__, 0x3fe80000, 0x0000000d, 0x3fe80000, 0x0000000d, 0x40130000, 0x00000002},			/* 0.75000=f(0.75000, v4.75000)*/
{64, 0,123,__LINE__, 0x3fe851eb, 0x851eb85f, 0x3fe851eb, 0x851eb85f, 0x40130a3d, 0x70a3d70c},			/* 0.76000=f(0.76000, v4.76000)*/
{64, 0,123,__LINE__, 0x3fe8a3d7, 0x0a3d70b1, 0x3fe8a3d7, 0x0a3d70b1, 0x4013147a, 0xe147ae16},			/* 0.77000=f(0.77000, v4.77000)*/
{64, 0,123,__LINE__, 0x3fe8f5c2, 0x8f5c2903, 0x3fe8f5c2, 0x8f5c2903, 0x40131eb8, 0x51eb8520},			/* 0.78000=f(0.78000, v4.78000)*/
{64, 0,123,__LINE__, 0x3fe947ae, 0x147ae155, 0x3fe947ae, 0x147ae155, 0x401328f5, 0xc28f5c2b},			/* 0.79000=f(0.79000, v4.79000)*/
{64, 0,123,__LINE__, 0x3fe99999, 0x999999a7, 0x3fe99999, 0x999999a7, 0x40133333, 0x33333335},			/* 0.80000=f(0.80000, v4.80000)*/
{64, 0,123,__LINE__, 0x3fe9eb85, 0x1eb851f9, 0x3fe9eb85, 0x1eb851f9, 0x40133d70, 0xa3d70a3f},			/* 0.81000=f(0.81000, v4.81000)*/
{64, 0,123,__LINE__, 0x3fea3d70, 0xa3d70a4b, 0x3fea3d70, 0xa3d70a4b, 0x401347ae, 0x147ae149},			/* 0.82000=f(0.82000, v4.82000)*/
{64, 0,123,__LINE__, 0x3fea8f5c, 0x28f5c29d, 0x3fea8f5c, 0x28f5c29d, 0x401351eb, 0x851eb854},			/* 0.83000=f(0.83000, v4.83000)*/
{64, 0,123,__LINE__, 0x3feae147, 0xae147aef, 0x3feae147, 0xae147aef, 0x40135c28, 0xf5c28f5e},			/* 0.84000=f(0.84000, v4.84000)*/
{64, 0,123,__LINE__, 0x3feb3333, 0x33333341, 0x3feb3333, 0x33333341, 0x40136666, 0x66666668},			/* 0.85000=f(0.85000, v4.85000)*/
{64, 0,123,__LINE__, 0x3feb851e, 0xb851eb93, 0x3feb851e, 0xb851eb93, 0x401370a3, 0xd70a3d72},			/* 0.86000=f(0.86000, v4.86000)*/
{64, 0,123,__LINE__, 0x3febd70a, 0x3d70a3e5, 0x3febd70a, 0x3d70a3e5, 0x40137ae1, 0x47ae147d},			/* 0.87000=f(0.87000, v4.87000)*/
{64, 0,123,__LINE__, 0x3fec28f5, 0xc28f5c37, 0x3fec28f5, 0xc28f5c37, 0x4013851e, 0xb851eb87},			/* 0.88000=f(0.88000, v4.88000)*/
{64, 0,123,__LINE__, 0x3fec7ae1, 0x47ae1489, 0x3fec7ae1, 0x47ae1489, 0x40138f5c, 0x28f5c291},			/* 0.89000=f(0.89000, v4.89000)*/
{64, 0,123,__LINE__, 0x3feccccc, 0xccccccdb, 0x3feccccc, 0xccccccdb, 0x40139999, 0x9999999b},			/* 0.90000=f(0.90000, v4.90000)*/
{64, 0,123,__LINE__, 0x3fed1eb8, 0x51eb852d, 0x3fed1eb8, 0x51eb852d, 0x4013a3d7, 0x0a3d70a6},			/* 0.91000=f(0.91000, v4.91000)*/
{64, 0,123,__LINE__, 0x3fed70a3, 0xd70a3d7f, 0x3fed70a3, 0xd70a3d7f, 0x4013ae14, 0x7ae147b0},			/* 0.92000=f(0.92000, v4.92000)*/
{64, 0,123,__LINE__, 0x3fedc28f, 0x5c28f5d1, 0x3fedc28f, 0x5c28f5d1, 0x4013b851, 0xeb851eba},			/* 0.93000=f(0.93000, v4.93000)*/
{64, 0,123,__LINE__, 0x3fee147a, 0xe147ae23, 0x3fee147a, 0xe147ae23, 0x4013c28f, 0x5c28f5c4},			/* 0.94000=f(0.94000, v4.94000)*/
{64, 0,123,__LINE__, 0x3fee6666, 0x66666675, 0x3fee6666, 0x66666675, 0x4013cccc, 0xcccccccf},			/* 0.95000=f(0.95000, v4.95000)*/
{64, 0,123,__LINE__, 0x3feeb851, 0xeb851ec7, 0x3feeb851, 0xeb851ec7, 0x4013d70a, 0x3d70a3d9},			/* 0.96000=f(0.96000, v4.96000)*/
{64, 0,123,__LINE__, 0x3fef0a3d, 0x70a3d719, 0x3fef0a3d, 0x70a3d719, 0x4013e147, 0xae147ae3},			/* 0.97000=f(0.97000, v4.97000)*/
{64, 0,123,__LINE__, 0x3fef5c28, 0xf5c28f6b, 0x3fef5c28, 0xf5c28f6b, 0x4013eb85, 0x1eb851ed},			/* 0.98000=f(0.98000, v4.98000)*/
{64, 0,123,__LINE__, 0x3fefae14, 0x7ae147bd, 0x3fefae14, 0x7ae147bd, 0x4013f5c2, 0x8f5c28f8},			/* 0.99000=f(0.99000, v4.99000)*/
{64, 0,123,__LINE__, 0x3ff00000, 0x00000007, 0x3ff00000, 0x00000007, 0x40140000, 0x00000002},			/* 1.00000=f(1.00000, v5.00000)*/
{64, 0,123,__LINE__, 0x3ff028f5, 0xc28f5c30, 0x3ff028f5, 0xc28f5c30, 0x40140a3d, 0x70a3d70c},			/* 1.01000=f(1.01000, v5.01000)*/
{64, 0,123,__LINE__, 0x3ff051eb, 0x851eb859, 0x3ff051eb, 0x851eb859, 0x4014147a, 0xe147ae16},			/* 1.02000=f(1.02000, v5.02000)*/
{64, 0,123,__LINE__, 0x3ff07ae1, 0x47ae1482, 0x3ff07ae1, 0x47ae1482, 0x40141eb8, 0x51eb8520},			/* 1.03000=f(1.03000, v5.03000)*/
{64, 0,123,__LINE__, 0x3ff0a3d7, 0x0a3d70ab, 0x3ff0a3d7, 0x0a3d70ab, 0x401428f5, 0xc28f5c2b},			/* 1.04000=f(1.04000, v5.04000)*/
{64, 0,123,__LINE__, 0x3ff0cccc, 0xccccccd4, 0x3ff0cccc, 0xccccccd4, 0x40143333, 0x33333335},			/* 1.05000=f(1.05000, v5.05000)*/
{64, 0,123,__LINE__, 0x3ff0f5c2, 0x8f5c28fd, 0x3ff0f5c2, 0x8f5c28fd, 0x40143d70, 0xa3d70a3f},			/* 1.06000=f(1.06000, v5.06000)*/
{64, 0,123,__LINE__, 0x3ff11eb8, 0x51eb8526, 0x3ff11eb8, 0x51eb8526, 0x401447ae, 0x147ae14a},			/* 1.07000=f(1.07000, v5.07000)*/
{64, 0,123,__LINE__, 0x3ff147ae, 0x147ae14f, 0x3ff147ae, 0x147ae14f, 0x401451eb, 0x851eb854},			/* 1.08000=f(1.08000, v5.08000)*/
{64, 0,123,__LINE__, 0x3ff170a3, 0xd70a3d78, 0x3ff170a3, 0xd70a3d78, 0x40145c28, 0xf5c28f5e},			/* 1.09000=f(1.09000, v5.09000)*/
{64, 0,123,__LINE__, 0x3ff19999, 0x999999a1, 0x3ff19999, 0x999999a1, 0x40146666, 0x66666668},			/* 1.10000=f(1.10000, v5.10000)*/
{64, 0,123,__LINE__, 0x3ff1c28f, 0x5c28f5ca, 0x3ff1c28f, 0x5c28f5ca, 0x401470a3, 0xd70a3d72},			/* 1.11000=f(1.11000, v5.11000)*/
{64, 0,123,__LINE__, 0x3ff1eb85, 0x1eb851f3, 0x3ff1eb85, 0x1eb851f3, 0x40147ae1, 0x47ae147d},			/* 1.12000=f(1.12000, v5.12000)*/
{64, 0,123,__LINE__, 0x3ff2147a, 0xe147ae1c, 0x3ff2147a, 0xe147ae1c, 0x4014851e, 0xb851eb87},			/* 1.13000=f(1.13000, v5.13000)*/
{64, 0,123,__LINE__, 0x3ff23d70, 0xa3d70a45, 0x3ff23d70, 0xa3d70a45, 0x40148f5c, 0x28f5c291},			/* 1.14000=f(1.14000, v5.14000)*/
{64, 0,123,__LINE__, 0x3ff26666, 0x6666666e, 0x3ff26666, 0x6666666e, 0x40149999, 0x9999999c},			/* 1.15000=f(1.15000, v5.15000)*/
{64, 0,123,__LINE__, 0x3ff28f5c, 0x28f5c297, 0x3ff28f5c, 0x28f5c297, 0x4014a3d7, 0x0a3d70a6},			/* 1.16000=f(1.16000, v5.16000)*/
{64, 0,123,__LINE__, 0x3ff2b851, 0xeb851ec0, 0x3ff2b851, 0xeb851ec0, 0x4014ae14, 0x7ae147b0},			/* 1.17000=f(1.17000, v5.17000)*/
{64, 0,123,__LINE__, 0x3ff2e147, 0xae147ae9, 0x3ff2e147, 0xae147ae9, 0x4014b851, 0xeb851eba},			/* 1.18000=f(1.18000, v5.18000)*/
{64, 0,123,__LINE__, 0x3ff30a3d, 0x70a3d712, 0x3ff30a3d, 0x70a3d712, 0x4014c28f, 0x5c28f5c4},			/* 1.19000=f(1.19000, v5.19000)*/
{64, 0,123,__LINE__, 0xbffb7812, 0xaeef4ba0, 0xc01921fb, 0x54442d18, 0xc00243f6, 0xa8885a30},			/* -1.71681=f(-6.28318, v-2.28318)*/
{64, 0,123,__LINE__, 0xbfdc0918, 0x40056e60, 0xc012d97c, 0x7f3321d2, 0xbfe6cbe3, 0xf9990e90},			/* -0.43805=f(-4.71238, v-0.71238)*/
{64, 0,123,__LINE__, 0xbfe21fb5, 0x4442d180, 0xc00921fb, 0x54442d18, 0x3feb7812, 0xaeef4ba0},			/* -0.56637=f(-3.14159, v0.85840)*/
{64, 0,123,__LINE__, 0xbff921fb, 0x54442d18, 0xbff921fb, 0x54442d18, 0x40036f02, 0x55dde974},			/* -1.57079=f(-1.57079, v2.42920)*/
{64, 0,123,__LINE__, 0x80000000, 0x00000000, 0x00000000, 0x00000000, 0x40100000, 0x00000000},			/* 0.00000=f(0.00000, v4.00000)*/
{64, 0,123,__LINE__, 0x3ff921fb, 0x54442d18, 0x3ff921fb, 0x54442d18, 0x4016487e, 0xd5110b46},			/* 1.57079=f(1.57079, v5.57079)*/
{64, 0,123,__LINE__, 0x400921fb, 0x54442d18, 0x400921fb, 0x54442d18, 0x401c90fd, 0xaa22168c},			/* 3.14159=f(3.14159, v7.14159)*/
{64, 0,123,__LINE__, 0x4012d97c, 0x7f3321d2, 0x4012d97c, 0x7f3321d2, 0x40216cbe, 0x3f9990e9},			/* 4.71238=f(4.71238, v8.71238)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc03e0000, 0x00000000, 0xc03a0000, 0x00000000},			/* -4.00000=f(-30.0000, v-26.0000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc03c4ccc, 0xcccccccd, 0xc0384ccc, 0xcccccccd},			/* -4.00000=f(-28.3000, v-24.3000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc03a9999, 0x9999999a, 0xc0369999, 0x9999999a},			/* -4.00000=f(-26.6000, v-22.6000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc038e666, 0x66666667, 0xc034e666, 0x66666667},			/* -4.00000=f(-24.9000, v-20.9000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc0373333, 0x33333334, 0xc0333333, 0x33333334},			/* -4.00000=f(-23.2000, v-19.2000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc0358000, 0x00000001, 0xc0318000, 0x00000001},			/* -4.00000=f(-21.5000, v-17.5000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc033cccc, 0xccccccce, 0xc02f9999, 0x9999999c},			/* -4.00000=f(-19.8000, v-15.8000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc0321999, 0x9999999b, 0xc02c3333, 0x33333336},			/* -4.00000=f(-18.1000, v-14.1000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc0306666, 0x66666668, 0xc028cccc, 0xccccccd0},			/* -4.00000=f(-16.4000, v-12.4000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc02d6666, 0x6666666a, 0xc0256666, 0x6666666a},			/* -4.00000=f(-14.7000, v-10.7000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc02a0000, 0x00000004, 0xc0220000, 0x00000004},			/* -4.00000=f(-13.0000, v-9.00000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc0269999, 0x9999999e, 0xc01d3333, 0x3333333c},			/* -4.00000=f(-11.3000, v-7.30000)*/
{64, 0,123,__LINE__, 0xc0100000, 0x00000000, 0xc0233333, 0x33333338, 0xc0166666, 0x66666670},			/* -4.00000=f(-9.60000, v-5.60000)*/
{64, 0,123,__LINE__, 0xbfb99999, 0x99999740, 0xc01f9999, 0x999999a3, 0xc00f3333, 0x33333346},			/* -0.00100=f(-7.90000, v-3.90000)*/
{64, 0,123,__LINE__, 0xbffccccc, 0xcccccca8, 0xc018cccc, 0xccccccd6, 0xc0019999, 0x999999ac},			/* -1.80000=f(-6.20000, v-2.20000)*/
{64, 0,123,__LINE__, 0xbfdfffff, 0xfffffc10, 0xc0120000, 0x00000009, 0xbfe00000, 0x00000048},			/* -0.50000=f(-4.50000, v-0.50000)*/
{64, 0,123,__LINE__, 0xbfd99999, 0x99999b40, 0xc0066666, 0x66666678, 0x3ff33333, 0x33333310},			/* -0.40000=f(-2.80000, v1.20000)*/
{64, 0,123,__LINE__, 0xbff19999, 0x999999bd, 0xbff19999, 0x999999bd, 0x40073333, 0x33333322},			/* -1.10000=f(-1.10000, v2.90000)*/
{64, 0,123,__LINE__, 0x3fe33333, 0x333332ec, 0x3fe33333, 0x333332ec, 0x40126666, 0x6666665e},			/* 0.60000=f(0.60000, v4.60000)*/
{64, 0,123,__LINE__, 0x40026666, 0x66666654, 0x40026666, 0x66666654, 0x40193333, 0x3333332a},			/* 2.30000=f(2.30000, v6.30000)*/
{64, 0,123,__LINE__, 0x400fffff, 0xffffffee, 0x400fffff, 0xffffffee, 0x401fffff, 0xfffffff7},			/* 4.00000=f(4.00000, v8.00000)*/
{64, 0,123,__LINE__, 0x4016cccc, 0xccccccc4, 0x4016cccc, 0xccccccc4, 0x40236666, 0x66666662},			/* 5.70000=f(5.70000, v9.70000)*/
{64, 0,123,__LINE__, 0x401d9999, 0x99999991, 0x401d9999, 0x99999991, 0x4026cccc, 0xccccccc8},			/* 7.40000=f(7.40000, v11.4000)*/
{64, 0,123,__LINE__, 0x40223333, 0x3333332f, 0x40223333, 0x3333332f, 0x402a3333, 0x3333332f},			/* 9.10000=f(9.10000, v13.1000)*/
{64, 0,123,__LINE__, 0x40259999, 0x99999995, 0x40259999, 0x99999995, 0x402d9999, 0x99999995},			/* 10.8000=f(10.8000, v14.8000)*/
{64, 0,123,__LINE__, 0x4028ffff, 0xfffffffb, 0x4028ffff, 0xfffffffb, 0x40307fff, 0xfffffffe},			/* 12.5000=f(12.5000, v16.5000)*/
{64, 0,123,__LINE__, 0x402c6666, 0x66666661, 0x402c6666, 0x66666661, 0x40323333, 0x33333330},			/* 14.2000=f(14.2000, v18.2000)*/
{64, 0,123,__LINE__, 0x402fcccc, 0xccccccc7, 0x402fcccc, 0xccccccc7, 0x4033e666, 0x66666664},			/* 15.9000=f(15.9000, v19.9000)*/
{64, 0,123,__LINE__, 0x40319999, 0x99999997, 0x40319999, 0x99999997, 0x40359999, 0x99999997},			/* 17.6000=f(17.6000, v21.6000)*/
{64, 0,123,__LINE__, 0x40334ccc, 0xccccccca, 0x40334ccc, 0xccccccca, 0x40374ccc, 0xccccccca},			/* 19.3000=f(19.3000, v23.3000)*/
{64, 0,123,__LINE__, 0x4034ffff, 0xfffffffd, 0x4034ffff, 0xfffffffd, 0x4038ffff, 0xfffffffd},			/* 21.0000=f(21.0000, v25.0000)*/
{64, 0,123,__LINE__, 0x4036b333, 0x33333330, 0x4036b333, 0x33333330, 0x403ab333, 0x33333330},			/* 22.7000=f(22.7000, v26.7000)*/
{64, 0,123,__LINE__, 0x40386666, 0x66666663, 0x40386666, 0x66666663, 0x403c6666, 0x66666663},			/* 24.4000=f(24.4000, v28.4000)*/
{64, 0,123,__LINE__, 0x403a1999, 0x99999996, 0x403a1999, 0x99999996, 0x403e1999, 0x99999996},			/* 26.1000=f(26.1000, v30.1000)*/
{64, 0,123,__LINE__, 0x403bcccc, 0xccccccc9, 0x403bcccc, 0xccccccc9, 0x403fcccc, 0xccccccc9},			/* 27.8000=f(27.8000, v31.8000)*/
{64, 0,123,__LINE__, 0x403d7fff, 0xfffffffc, 0x403d7fff, 0xfffffffc, 0x4040bfff, 0xfffffffe},			/* 29.5000=f(29.5000, v33.5000)*/
0,};
test_fmod(m)   {run_vector_1(m,fmod_vec,(char *)(fmod),"fmod","ddd");   }	
